#include "motores.h"

static int limitarPWM(int pwm) {
    if (pwm > PWM_MAX) return PWM_MAX;
    if (pwm < -PWM_MAX) return -PWM_MAX;
    return pwm;
}

void inicializarMotores() {
    pinMode(PIN_NSLEEP, OUTPUT);

    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);

    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);

    digitalWrite(PIN_NSLEEP, HIGH);

    detenerMotores();

    Serial.println("[MOTORES] Inicializados");
}

void setMotorIzquierdo(int pwm) {
    pwm = limitarPWM(pwm);

    if (pwm > 0) {
        analogWrite(AIN1, pwm);
        analogWrite(AIN2, 0);
    } 
    else if (pwm < 0) {
        analogWrite(AIN1, 0);
        analogWrite(AIN2, -pwm);
    } 
    else {
        analogWrite(AIN1, 0);
        analogWrite(AIN2, 0);
    }
}

void setMotorDerecho(int pwm) {
    pwm = limitarPWM(pwm);

    if (pwm > 0) {
        analogWrite(BIN1, pwm);
        analogWrite(BIN2, 0);
    } 
    else if (pwm < 0) {
        analogWrite(BIN1, 0);
        analogWrite(BIN2, -pwm);
    } 
    else {
        analogWrite(BIN1, 0);
        analogWrite(BIN2, 0);
    }
}

void setMotores(int pwmIzquierdo, int pwmDerecho) {
    setMotorIzquierdo(pwmIzquierdo);
    setMotorDerecho(pwmDerecho);
}

void detenerMotores() {
    setMotorIzquierdo(0);
    setMotorDerecho(0);
}

void frenarSuave() {
    setMotores(120, 120);
    delay(25);

    setMotores(80, 80);
    delay(25);

    setMotores(40, 40);
    delay(20);

    detenerMotores();
}

void frenarSuaveDesde(int pwmInicial) {
    for (int pwm = pwmInicial; pwm >= 0; pwm -= 40) {
        setMotores(pwm, pwm);
        delay(10);
    }

    detenerMotores();
}

// =====================================================
// FUNCIONES DE PRUEBA
// =====================================================

void pruebaAvanzar() {
    Serial.println("[MOTORES] Prueba avanzar");
    setMotores(PWM_BASE_AVANCE, PWM_BASE_AVANCE);
}

void pruebaRetroceder() {
    Serial.println("[MOTORES] Prueba retroceder");
    setMotores(-PWM_BASE_AVANCE, -PWM_BASE_AVANCE);
}

void pruebaGirarIzquierda() {
    Serial.println("[MOTORES] Prueba girar izquierda");
    setMotores(-PWM_BASE_GIRO, PWM_BASE_GIRO);
}

void pruebaGirarDerecha() {
    Serial.println("[MOTORES] Prueba girar derecha");
    setMotores(PWM_BASE_GIRO, -PWM_BASE_GIRO);
}