#include "config.h"
#include "imu.h"

bool reporteAutomatico = false;
unsigned long ultimoReporte = 0;
const unsigned long INTERVALO_REPORTE_MS = 500;

void mostrarAyuda() {
    Serial.println();
    Serial.println("===== PRUEBA IMU =====");
    Serial.println("IMU   -> imprimir datos IMU");
    Serial.println("YAW0  -> resetear yaw relativo");
    Serial.println("CAL   -> recalibrar giroscopio");
    Serial.println("AUTO  -> activar/desactivar reporte automatico");
    Serial.println("AYUDA -> mostrar ayuda");
    Serial.println("======================");
    Serial.println();
}

void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    delay(1000);

    Serial.println("Iniciando prueba IMU...");

    bool ok = inicializarIMU();

    if (ok) {
        Serial.println("[TEST] IMU lista");
    } else {
        Serial.println("[TEST] Error inicializando IMU");
    }

    mostrarAyuda();
}

void loop() {
    actualizarIMU();

    if (reporteAutomatico && millis() - ultimoReporte >= INTERVALO_REPORTE_MS) {
        ultimoReporte = millis();
        imprimirIMU();
    }

    if (!Serial.available()) {
        return;
    }

    String comando = Serial.readStringUntil('\n');
    comando.trim();
    comando.toUpperCase();

    if (comando == "IMU") {
        imprimirIMU();
    }

    else if (comando == "YAW0") {
        resetYaw();
        Serial.println("[TEST] Yaw reseteado");
    }

    else if (comando == "CAL") {
        calibrarIMU();
        resetYaw();
        Serial.println("[TEST] IMU recalibrada");
    }

    else if (comando == "AUTO") {
        reporteAutomatico = !reporteAutomatico;
        Serial.print("[TEST] Reporte automatico: ");
        Serial.println(reporteAutomatico ? "ON" : "OFF");
    }

    else if (comando == "AYUDA") {
        mostrarAyuda();
    }

    else {
        Serial.println("[TEST] Comando no reconocido");
    }
}