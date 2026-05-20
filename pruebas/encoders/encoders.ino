#include "config.h"
#include "encoders.h"

unsigned long ultimoReporte = 0;
const unsigned long INTERVALO_REPORTE_MS = 1000;

void mostrarAyuda() {
    Serial.println();
    Serial.println("===== PRUEBA ENCODERS =====");
    Serial.println("E   -> imprimir encoders");
    Serial.println("ER  -> resetear encoders");
    Serial.println("AUTO -> activar/desactivar reporte automatico");
    Serial.println("AYUDA -> mostrar ayuda");
    Serial.println("===========================");
    Serial.println();
}

bool reporteAutomatico = false;

void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    delay(1000);

    Serial.println("Iniciando prueba de encoders...");

    inicializarEncoders();

    mostrarAyuda();
}

void loop() {
    if (reporteAutomatico && millis() - ultimoReporte >= INTERVALO_REPORTE_MS) {
        ultimoReporte = millis();
        imprimirEncoders();
    }

    if (!Serial.available()) {
        return;
    }

    String comando = Serial.readStringUntil('\n');
    comando.trim();
    comando.toUpperCase();

    if (comando == "E") {
        imprimirEncoders();
    }

    else if (comando == "ER") {
        resetEncoders();
        Serial.println("[TEST] Encoders reseteados");
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