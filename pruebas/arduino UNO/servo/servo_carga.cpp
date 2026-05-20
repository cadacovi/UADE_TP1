#include "servo_carga.h"

static Servo servoCarga;
static int anguloActualServo = SERVO_ANGULO_CERRADO;
static bool servoInicializado = false;

void inicializarServoCarga() {
    servoCarga.attach(SERVO_PIN);

    servoInicializado = true;

    cerrarCarga();

    Serial.println("[SERVO] Servo de carga inicializado");
}

void abrirCarga() {
    if (!servoInicializado) {
        Serial.println("[SERVO] Error: servo no inicializado");
        return;
    }

    servoCarga.write(SERVO_ANGULO_ABIERTO);
    anguloActualServo = SERVO_ANGULO_ABIERTO;

    Serial.println("[SERVO] Carga abierta");
}

void cerrarCarga() {
    servoCarga.write(SERVO_ANGULO_CERRADO);
    anguloActualServo = SERVO_ANGULO_CERRADO;

    Serial.println("[SERVO] Carga cerrada");
}

void descargarCarga() {
    if (!servoInicializado) {
        Serial.println("[SERVO] Error: servo no inicializado");
        return;
    }

    Serial.println("[SERVO] Descargando carga...");

    abrirCarga();
    delay(TIEMPO_APERTURA_SERVO_MS);
    cerrarCarga();

    Serial.println("[SERVO] Descarga completada");
}

void imprimirEstadoServo() {
    Serial.println();
    Serial.println("===== SERVO CARGA =====");

    Serial.print("Angulo actual: ");
    Serial.println(anguloActualServo);

    Serial.print("Inicializado: ");
    Serial.println(servoInicializado ? "SI" : "NO");

    Serial.println("=======================");
    Serial.println();
    }