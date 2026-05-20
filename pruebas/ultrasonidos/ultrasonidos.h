#ifndef ULTRASONIDOS_H
#define ULTRASONIDOS_H

#include <Arduino.h>
#include "config.h"

void inicializarUltrasonidos();

float leerFrontalMm();
float leerIzquierdoMm();
float leerDerechoMm();

float leerFrontalFiltradoMm();
float leerIzquierdoFiltradoMm();
float leerDerechoFiltradoMm();

bool hayObstaculoFrontal();
bool hayObstaculoIzquierdo();
bool hayObstaculoDerecho();

void imprimirUltrasonidos();

#endif