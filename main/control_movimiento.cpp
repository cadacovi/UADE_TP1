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
    Serial.println("[CONTROL] Avanzando distancia controlada");

    resetEncoders();

    float yawObjetivo = obtenerYawRelativo();
    unsigned long tiempoInicio = millis();

    while (true) {
        actualizarIMU();

        float distanciaActual = obtenerDistanciaPromedioMm();

        if (distanciaActual >= distanciaObjetivoMm - TOLERANCIA_DISTANCIA_MM) {
        break;
        }

        if (millis() - tiempoInicio > TIMEOUT_AVANCE_CELDA_MS) {
        detenerMotores();
        Serial.println("[CONTROL] Error: timeout avance");
        return false;
        }

        if (hayObstaculoFrontal()) {
        detenerMotores();
        Serial.println("[CONTROL] Obstaculo frontal detectado");
        return false;
        }

        float errorRuedas = obtenerErrorRuedasMm();
        float errorYaw = yawObjetivo - obtenerYawRelativo();

        float correccion = (KP_ENCODER_AVANCE * errorRuedas) +
                        (KP_YAW_AVANCE * errorYaw);

        int pwmIzquierdo = limitarPWMControl(
        PWM_BASE_AVANCE - correccion,
        PWM_AVANCE_MIN,
        PWM_AVANCE_MAX
        );

        int pwmDerecho = limitarPWMControl(
        PWM_BASE_AVANCE + correccion,
        PWM_AVANCE_MIN,
        PWM_AVANCE_MAX
        );

        setMotores(pwmIzquierdo, pwmDerecho);

        delay(10);
    }

    frenarSuave();

    Serial.println("[CONTROL] Avance completado");
    return true;
    }

    bool avanzarUnaCelda() {
    return avanzarDistanciaMm(TAM_CELDA_MM);
}

bool girarAngulo(float grados) {
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

        if (error <= TOLERANCIA_GIRO_GRADOS) {
        break;
        }

        if (millis() - tiempoInicio > TIMEOUT_GIRO_MS) {
        detenerMotores();
        Serial.println("[CONTROL] Error: timeout giro");
        return false;
        }

        float pwm = KP_GIRO * error;
        int pwmGiro = limitarPWMControl(pwm, PWM_GIRO_MIN, PWM_GIRO_MAX);

        if (sentido > 0) {
        // Giro derecha
        setMotores(pwmGiro, -pwmGiro);
        } else {
        // Giro izquierda
        setMotores(-pwmGiro, pwmGiro);
        }

        delay(10);
    }

    detenerMotores();

    Serial.println("[CONTROL] Giro completado");
    return true;
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