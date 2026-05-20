#ifndef ENCODERS_H
#define ENCODERS_H

#include <Arduino.h>
#include "config.h"

void inicializarEncoders();
void resetEncoders();

long obtenerPulsosIzquierdo();
long obtenerPulsosDerecho();

float obtenerDistanciaIzquierdaMm();
float obtenerDistanciaDerechaMm();
float obtenerDistanciaPromedioMm();
float obtenerErrorRuedasMm();

void imprimirEncoders();

#endif