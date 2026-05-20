#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLUMNAS, LCD_FILAS);

void mostrarAyuda() {
    Serial.println();
    Serial.println("===== PRUEBA LCD 1602 I2C =====");
    Serial.println("HOLA     -> mensaje de prueba");
    Serial.println("IDLE     -> estado IDLE");
    Serial.println("NAV      -> estado navegando");
    Serial.println("OBS      -> obstaculo");
    Serial.println("ERR      -> error");
    Serial.println("CLEAR    -> limpiar pantalla");
    Serial.println("AYUDA    -> mostrar ayuda");
    Serial.println("================================");
    Serial.println();
}

void mostrarMensaje(const char* linea1, const char* linea2) {
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print(linea1);

    lcd.setCursor(0, 1);
    lcd.print(linea2);
}

void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    delay(1000);

    Serial.println("Iniciando prueba LCD 1602 I2C...");

    Wire.begin();

    lcd.init();
    lcd.backlight();

    mostrarMensaje("LCD 1602 I2C", "Prueba OK");

    mostrarAyuda();
}

void loop() {
    if (!Serial.available()) {
        return;
    }

    String comando = Serial.readStringUntil('\n');
    comando.trim();
    comando.toUpperCase();

    if (comando == "HOLA") {
        mostrarMensaje("Hola Carlos", "LCD funcionando");
    }

    else if (comando == "IDLE") {
        mostrarMensaje("IDLE", "Esperando orden");
    }

    else if (comando == "NAV") {
        mostrarMensaje("NAVEGANDO", "Pos 0,1 Dst 0,3");
    }

    else if (comando == "OBS") {
        mostrarMensaje("OBSTACULO", "Recalculando...");
    }

    else if (comando == "ERR") {
        mostrarMensaje("ERROR", "Revisar robot");
    }

    else if (comando == "CLEAR") {
        lcd.clear();
    }

    else if (comando == "AYUDA") {
        mostrarAyuda();
    }

    else {
        Serial.println("[LCD] Comando no reconocido");
        mostrarMensaje("Comando inval.", "Usa AYUDA");
    }
}