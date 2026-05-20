#include "encoders.h"

volatile long pulsosIzquierdo = 0;
volatile long pulsosDerecho = 0;

portMUX_TYPE muxEncoders = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR encoderIzquierdoISR() {
    portENTER_CRITICAL_ISR(&muxEncoders);
    pulsosIzquierdo++;
    portEXIT_CRITICAL_ISR(&muxEncoders);
}

void IRAM_ATTR encoderDerechoISR() {
    portENTER_CRITICAL_ISR(&muxEncoders);
    pulsosDerecho++;
    portEXIT_CRITICAL_ISR(&muxEncoders);
}

void inicializarEncoders() {
    pinMode(ENCODER_LEFT, INPUT_PULLUP);
    pinMode(ENCODER_RIGHT, INPUT_PULLUP);

    resetEncoders();

    attachInterrupt(digitalPinToInterrupt(ENCODER_LEFT), encoderIzquierdoISR, RISING);
    attachInterrupt(digitalPinToInterrupt(ENCODER_RIGHT), encoderDerechoISR, RISING);

    Serial.println("[ENCODERS] Inicializados");
}

void resetEncoders() {
    portENTER_CRITICAL(&muxEncoders);
    pulsosIzquierdo = 0;
    pulsosDerecho = 0;
    portEXIT_CRITICAL(&muxEncoders);
}

long obtenerPulsosIzquierdo() {
    long copia;

    portENTER_CRITICAL(&muxEncoders);
    copia = pulsosIzquierdo;
    portEXIT_CRITICAL(&muxEncoders);

    return copia;
}

long obtenerPulsosDerecho() {
    long copia;

    portENTER_CRITICAL(&muxEncoders);
    copia = pulsosDerecho;
    portEXIT_CRITICAL(&muxEncoders);

    return copia;
}

float obtenerDistanciaIzquierdaMm() {
    return obtenerPulsosIzquierdo() / PULSOS_POR_MM;
}

float obtenerDistanciaDerechaMm() {
    return obtenerPulsosDerecho() / PULSOS_POR_MM;
}

float obtenerDistanciaPromedioMm() {
    float izquierda = obtenerDistanciaIzquierdaMm();
    float derecha = obtenerDistanciaDerechaMm();

    return (izquierda + derecha) / 2.0;
}

float obtenerErrorRuedasMm() {
    return obtenerDistanciaIzquierdaMm() - obtenerDistanciaDerechaMm();
}

void imprimirEncoders() {
    Serial.println();
    Serial.println("===== ENCODERS =====");

    Serial.print("Pulsos izquierdo: ");
    Serial.println(obtenerPulsosIzquierdo());

    Serial.print("Pulsos derecho: ");
    Serial.println(obtenerPulsosDerecho());

    Serial.print("Distancia izquierda mm: ");
    Serial.println(obtenerDistanciaIzquierdaMm());

    Serial.print("Distancia derecha mm: ");
    Serial.println(obtenerDistanciaDerechaMm());

    Serial.print("Distancia promedio mm: ");
    Serial.println(obtenerDistanciaPromedioMm());

    Serial.print("Error ruedas mm: ");
    Serial.println(obtenerErrorRuedasMm());

    Serial.println("====================");
    Serial.println();
}