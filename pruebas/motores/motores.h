#ifndef MOTORES_H
#define MOTORES_H

#include <Arduino.h>
#include "config.h"

void inicializarMotores();

void setMotorIzquierdo(int pwm);
void setMotorDerecho(int pwm);
void setMotores(int pwmIzquierdo, int pwmDerecho);

void detenerMotores();
void frenarSuave();

void pruebaAvanzar();
void pruebaRetroceder();
void pruebaGirarIzquierda();
void pruebaGirarDerecha();

#endif