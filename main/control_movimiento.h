#ifndef CONTROL_MOVIMIENTO_H
#define CONTROL_MOVIMIENTO_H

#include <Arduino.h>
#include "config.h"
#include "tipos.h"

bool avanzarDistanciaMm(float distanciaObjetivoMm);
bool avanzarUnaCelda();

bool girarAngulo(float grados);
bool girar90Derecha();
bool girar90Izquierda();
bool girar180();

ResultadoAvance avanzarDistanciaResultado(float distanciaObjetivoMm);
ResultadoGiro girarAnguloResultado(float grados);

#endif