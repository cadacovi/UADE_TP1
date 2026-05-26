#include "control_movimiento.h"
#include "motores.h"
#include "encoders.h"
#include "imu.h"
#include "ultrasonidos.h"

static int limitarPWMControl(float pwm, int pwmMin, int pwmMax) {
    int valor = (int)pwm;

    if (valor > pwmMax) return pwmMax;
    if (valor < -pwmMax) return -pwmMax;

    if (valor > 0 && valor < pwmMin) return pwmMin;
    if (valor < 0 && valor > -pwmMin) return -pwmMin;

    return valor;
}

bool avanzarDistanciaMm(float distanciaObjetivoMm) {
    ResultadoAvance r = avanzarDistanciaResultado(distanciaObjetivoMm);
    return r.exito;
}

ResultadoAvance avanzarDistanciaResultado(float distanciaObjetivoMm) {
    ResultadoAvance resultado;
    resultado.exito = false;
    resultado.obstaculoDetectado = false;
    resultado.timeout = false;
    resultado.distanciaRecorridaMm = 0.0;

    resetEncoders();

    float yawObjetivo = obtenerYawRelativo();
    unsigned long tiempoInicio = millis();

    unsigned long ultimaLecturaUS = 0;

    int pwmBase = PWM_BASE_AVANCE;

    bool reversa = distanciaObjetivoMm < 0;

    Serial.println(reversa ? "[CONTROL] Retrocediendo distancia controlada"
                       : "[CONTROL] Avanzando distancia controlada");

    float distanciaObjetivo = abs(distanciaObjetivoMm);

    while (true) {
        actualizarIMU();

        float distanciaActual = obtenerDistanciaPromedioMm();
        resultado.distanciaRecorridaMm = distanciaActual;

        if (distanciaActual >= distanciaObjetivo - TOLERANCIA_DISTANCIA_MM) {
            resultado.exito = true;
            break;
        }

        if (millis() - tiempoInicio > TIMEOUT_AVANCE_CELDA_MS) {
            resultado.timeout = true;
            resultado.exito = false;
            break;
        }

        float restante = distanciaObjetivo - distanciaActual;

        if (restante < DISTANCIA_FRENADO_MM) {
            pwmBase = PWM_FINAL_AVANCE;
        }

        if (millis() - ultimaLecturaUS >= 80 && !reversa) {
            ultimaLecturaUS = millis();

            if (leerFrontalFiltradoRapidoMm() <= DIST_OBSTACULO_FRENTE_MM) {
                resultado.obstaculoDetectado = true;
                resultado.exito = false;
                break;
            }
        }

        float errorRuedas = obtenerErrorRuedasMm();
        float errorYaw = yawObjetivo - obtenerYawRelativo();

        float correccion = (KP_ENCODER_AVANCE * errorRuedas) +
                        (KP_YAW_AVANCE * errorYaw);

        int pwmIzquierdo = limitarPWMControl(
        pwmBase - correccion,
        PWM_AVANCE_MIN,
        PWM_AVANCE_MAX
        );

        int pwmDerecho = limitarPWMControl(
        pwmBase + correccion,
        PWM_AVANCE_MIN,
        PWM_AVANCE_MAX
        );

        if (reversa) {
            pwmIzquierdo = -pwmIzquierdo;
            pwmDerecho = -pwmDerecho;
        }

        setMotores(pwmIzquierdo, pwmDerecho);

        // delay(10);
    }

    detenerMotores();
    // frenarSuaveDesde(PWM_BASE_AVANCE);

    delay(100);

    actualizarIMU();

    float errorFinalYaw = yawObjetivo - obtenerYawRelativo();


    if (abs(errorFinalYaw) > TOLERANCIA_CORRECCION_YAW &&
        abs(errorFinalYaw) < MAX_CORRECCION_YAW) {
        girarAngulo(-errorFinalYaw);
    } //correccion final usando IMU

    resultado.distanciaRecorridaMm = obtenerDistanciaPromedioMm();

    if (resultado.exito) {
        Serial.println("[CONTROL] Avance completado");
    } else if (resultado.obstaculoDetectado) {
        Serial.println("[CONTROL] Avance detenido por obstaculo");
    } else if (resultado.timeout) {
        Serial.println("[CONTROL] Error: timeout avance");
    } else {
        Serial.println("[CONTROL] Avance fallido");
    }

    return resultado;
}

bool avanzarUnaCelda() {
    return avanzarDistanciaMm(TAM_CELDA_MM);
}

bool girarAngulo(float grados) {
    ResultadoGiro r = girarAnguloResultado(grados);
    return r.exito;
}

ResultadoGiro girarAnguloResultado(float grados){
    ResultadoGiro resultado;
    resultado.exito = false;
    resultado.timeout = false;

    Serial.print("[CONTROL] Girando grados: ");
    Serial.println(grados);

    resetYaw();

    int sentido = grados >= 0 ? 1 : -1;
    float objetivo = abs(grados);

    unsigned long tiempoInicio = millis();

    while (true) {
        actualizarIMU();

        float yawActual = abs(obtenerYawRelativo());
        float error = objetivo - yawActual;

        resultado.errorFinalGrados = error;

        if (error <= TOLERANCIA_GIRO_GRADOS) {
            resultado.exito = true;
            break;
        }

        if (millis() - tiempoInicio > TIMEOUT_GIRO_MS) {
            resultado.timeout = true;
            resultado.exito = false;
            break;
        }

        float pwm = KP_GIRO * error;
        int pwmGiro = limitarPWMControl(pwm, PWM_GIRO_MIN, PWM_GIRO_MAX);

        if (objetivo >= 45 && error < 20) {
            pwmGiro = PWM_FINAL_GIRO;
        }
        // Suavizar final de giro para mejor precision

        if ((objetivo < 45 && objetivo >= 10) && error < 5) {
            pwmGiro = PWM_FINAL_GIRO;
        }
        // Para giros pequeños, usar potencia minima para evitar overshoot

        if (sentido > 0) {
        // Giro derecha
        setMotores(pwmGiro, -pwmGiro);
        } else {
        // Giro izquierda
        setMotores(-pwmGiro, pwmGiro);
        }
        // delay(10);
    }

    detenerMotores();
    delay(100);

    actualizarIMU();

    float yawFinal = abs(obtenerYawRelativo());
    resultado.errorFinalGrados = objetivo - yawFinal;

    if (resultado.exito) {
        Serial.print("[CONTROL] Giro completado. Error final: ");
        Serial.println(resultado.errorFinalGrados);
    } else if (resultado.timeout) {
        Serial.print("[CONTROL] Error: timeout giro. Error final: ");
        Serial.println(resultado.errorFinalGrados);
    } else {
        Serial.println("[CONTROL] Giro fallido");
    }
    
    return resultado;
}

bool girar90Derecha() {
    return girarAngulo(90.0);
}

bool girar90Izquierda() {
    return girarAngulo(-90.0);
}

bool girar180() {
    return girarAngulo(180.0);
}