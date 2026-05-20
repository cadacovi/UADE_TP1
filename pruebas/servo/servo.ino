#include "config.h"
#include "servo_carga.h"

void mostrarAyuda() {
    Serial.println();
    Serial.println("===== PRUEBA SERVO =====");
    Serial.println("SERVO  -> imprimir estado");
    Serial.println("ABRIR  -> abrir carga");
    Serial.println("CERRAR -> cerrar carga");
    Serial.println("D      -> descargar");
    Serial.println("AYUDA  -> mostrar ayuda");
    Serial.println("========================");
    Serial.println();
}

void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    delay(1000);

    Serial.println("Iniciando prueba servo...");

    inicializarServoCarga();

    mostrarAyuda();
}

void loop() {
    if (!Serial.available()) {
        return;
    }

    String comando = Serial.readStringUntil('\n');
    comando.trim();
    comando.toUpperCase();

    if (comando == "SERVO") {
        imprimirEstadoServo();
    }

    else if (comando == "ABRIR") {
        abrirCarga();
    }

    else if (comando == "CERRAR") {
        cerrarCarga();
    }

    else if (comando == "D") {
        descargarCarga();
    }

    else if (comando == "AYUDA") {
        mostrarAyuda();
    }

    else {
        Serial.println("[TEST] Comando no reconocido");
    }
}