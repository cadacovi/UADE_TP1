#include "config.h"
#include "ultrasonidos.h"

bool reporteAutomatico = false;
unsigned long ultimoReporte = 0;
const unsigned long INTERVALO_REPORTE_MS = 700;

void mostrarAyuda() {
    Serial.println();
    Serial.println("===== PRUEBA ULTRASONIDOS =====");
    Serial.println("U     -> imprimir ultrasonidos filtrados");
    Serial.println("UF    -> leer frontal filtrado");
    Serial.println("UI    -> leer izquierdo filtrado");
    Serial.println("UD    -> leer derecho filtrado");
    Serial.println("RAW   -> leer los 3 sin filtro");
    Serial.println("AUTO  -> activar/desactivar reporte automatico");
    Serial.println("AYUDA -> mostrar ayuda");
    Serial.println("================================");
    Serial.println();
}

void imprimirRaw() {
    Serial.println();
    Serial.println("===== ULTRASONIDOS RAW =====");

    Serial.print("Frontal mm: ");
    Serial.println(leerFrontalMm());

    Serial.print("Izquierdo mm: ");
    Serial.println(leerIzquierdoMm());

    Serial.print("Derecho mm: ");
    Serial.println(leerDerechoMm());

    Serial.println("============================");
    Serial.println();
}

void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    delay(1000);

    Serial.println("Iniciando prueba ultrasonidos...");

    inicializarUltrasonidos();

    mostrarAyuda();
}

void loop() {
    if (reporteAutomatico && millis() - ultimoReporte >= INTERVALO_REPORTE_MS) {
        ultimoReporte = millis();
        imprimirUltrasonidos();
    }

    if (!Serial.available()) {
        return;
    }

    String comando = Serial.readStringUntil('\n');
    comando.trim();
    comando.toUpperCase();

    if (comando == "U") {
        imprimirUltrasonidos();
    }

    else if (comando == "UF") {
        Serial.print("[US] Frontal filtrado mm: ");
        Serial.println(leerFrontalFiltradoMm());
    }

    else if (comando == "UI") {
        Serial.print("[US] Izquierdo filtrado mm: ");
        Serial.println(leerIzquierdoFiltradoMm());
    }

    else if (comando == "UD") {
        Serial.print("[US] Derecho filtrado mm: ");
        Serial.println(leerDerechoFiltradoMm());
    }

    else if (comando == "RAW") {
        imprimirRaw();
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