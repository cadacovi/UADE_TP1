#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

bool inicializarIMU();
void calibrarIMU();
void actualizarIMU();

void resetYaw();
float obtenerYawRelativo();
float obtenerGyroZ();

void imprimirIMU();

#endif