#ifndef SERVO_CARGA_H
#define SERVO_CARGA_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"

void inicializarServoCarga();

void abrirCarga();
void cerrarCarga();
void descargarCarga();

void imprimirEstadoServo();

#endif