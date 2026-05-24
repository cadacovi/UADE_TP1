#include "ultrasonidos.h"

void inicializarUltrasonidos() {
    pinMode(TRIG_FRONT, OUTPUT);
    pinMode(ECHO_FRONT, INPUT);

    pinMode(TRIG_LEFT, OUTPUT);
    pinMode(ECHO_LEFT, INPUT);

    pinMode(TRIG_RIGHT, OUTPUT);
    pinMode(ECHO_RIGHT, INPUT);

    digitalWrite(TRIG_FRONT, LOW);
    digitalWrite(TRIG_LEFT, LOW);
    digitalWrite(TRIG_RIGHT, LOW);

    Serial.println("[US] Ultrasonidos inicializados");
}

static float medirDistanciaMm(int trigPin, int echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(3);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    unsigned long duracion = pulseIn(echoPin, HIGH, 25000UL);

    if (duracion == 0) {
        return DIST_ULTRASONICO_MAX_MM;
    }

    // Velocidad del sonido aprox: 0.343 mm/us
    float distanciaMm = (duracion * 0.343) / 2.0;

    if (distanciaMm <= 0 || distanciaMm > DIST_ULTRASONICO_MAX_MM) {
        return DIST_ULTRASONICO_MAX_MM;
    }

    return distanciaMm;
}

static void ordenar(float datos[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
        if (datos[j] > datos[j + 1]) {
            float temp = datos[j];
            datos[j] = datos[j + 1];
            datos[j + 1] = temp;
        }
        }
    }
}

static float medirDistanciaFiltradaMm(int trigPin, int echoPin) {
    float muestras[ULTRASONICO_MUESTRAS];

    for (int i = 0; i < ULTRASONICO_MUESTRAS; i++) {
        muestras[i] = medirDistanciaMm(trigPin, echoPin);
        delay(15);
    }

    ordenar(muestras, ULTRASONICO_MUESTRAS);

    // Mediana
    return muestras[ULTRASONICO_MUESTRAS / 2];
}

static float medirDistanciaFiltradaRapidaMm(int trigPin, int echoPin) {
    const int muestrasRapidas = 3;
    float muestras[muestrasRapidas];

    for (int i = 0; i < muestrasRapidas; i++) {
        muestras[i] = medirDistanciaMm(trigPin, echoPin);
        delay(4);
    }

    ordenar(muestras, muestrasRapidas);

    return muestras[muestrasRapidas / 2]; // mediana de 3
}

float leerFrontalMm() {
    return medirDistanciaMm(TRIG_FRONT, ECHO_FRONT);
}

float leerIzquierdoMm() {
    return medirDistanciaMm(TRIG_LEFT, ECHO_LEFT);
}

float leerDerechoMm() {
    return medirDistanciaMm(TRIG_RIGHT, ECHO_RIGHT);
}

float leerFrontalFiltradoMm() {
    return medirDistanciaFiltradaMm(TRIG_FRONT, ECHO_FRONT);
}

float leerFrontalFiltradoRapidoMm() {
    return medirDistanciaFiltradaRapidaMm(TRIG_FRONT, ECHO_FRONT);
}

float leerIzquierdoFiltradoMm() {
    return medirDistanciaFiltradaMm(TRIG_LEFT, ECHO_LEFT);
}

float leerDerechoFiltradoMm() {
    return medirDistanciaFiltradaMm(TRIG_RIGHT, ECHO_RIGHT);
}

bool hayObstaculoFrontal() {
    return leerFrontalFiltradoMm() <= DIST_OBSTACULO_FRENTE_MM;
}

bool hayObstaculoIzquierdo() {
    return leerIzquierdoFiltradoMm() <= DIST_OBSTACULO_LATERAL_MM;
}

bool hayObstaculoDerecho() {
    return leerDerechoFiltradoMm() <= DIST_OBSTACULO_LATERAL_MM;
}

void imprimirUltrasonidos() {
    float frontal = leerFrontalFiltradoMm();
    float izquierdo = leerIzquierdoFiltradoMm();
    float derecho = leerDerechoFiltradoMm();

    Serial.println();
    Serial.println("===== ULTRASONIDOS =====");

    Serial.print("Frontal mm: ");
    Serial.println(frontal);

    Serial.print("Izquierdo mm: ");
    Serial.println(izquierdo);

    Serial.print("Derecho mm: ");
    Serial.println(derecho);

    Serial.print("Obstaculo frontal: ");
    Serial.println(frontal <= DIST_OBSTACULO_FRENTE_MM ? "SI" : "NO");

    Serial.print("Obstaculo izquierdo: ");
    Serial.println(izquierdo <= DIST_OBSTACULO_LATERAL_MM ? "SI" : "NO");

    Serial.print("Obstaculo derecho: ");
    Serial.println(derecho <= DIST_OBSTACULO_LATERAL_MM ? "SI" : "NO");

    Serial.println("========================");
    Serial.println();
}