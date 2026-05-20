#include "config.h"
#include "motores.h"

void mostrarAyuda() {
    Serial.println();
    Serial.println("===== PRUEBA MOTORES =====");
    Serial.println("F  -> avanzar");
    Serial.println("B  -> retroceder");
    Serial.println("L  -> girar izquierda");
    Serial.println("R  -> girar derecha");
    Serial.println("S  -> detener");
    Serial.println("1  -> avanzar suave");
    Serial.println("2  -> retroceder suave");
    Serial.println("==========================");
    Serial.println();
}

void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    delay(1000);

    Serial.println("Iniciando prueba de motores...");

    inicializarMotores();

    mostrarAyuda();
}

void loop() {
    if (!Serial.available()) {
        return;
    }

    String comando = Serial.readStringUntil('\n');
    comando.trim();
    comando.toUpperCase();

    if (comando == "F") {
        pruebaAvanzar();
    }

    else if (comando == "B") {
        pruebaRetroceder();
    }

    else if (comando == "L") {
        pruebaGirarIzquierda();
    }

    else if (comando == "R") {
        pruebaGirarDerecha();
    }

    else if (comando == "S") {
        detenerMotores();
        Serial.println("[TEST] Motores detenidos");
    }

    else if (comando == "1") {
        setMotores(255, 255);
        Serial.println("[TEST] Avance suave PWM 100");
    }

    else if (comando == "2") {
        setMotores(-255, -255);
        Serial.println("[TEST] Retroceso suave PWM 100");
    }
    

    else if (comando == "AYUDA") {
        mostrarAyuda();
    }

    else {
        Serial.println("[TEST] Comando no reconocido");
    }
}