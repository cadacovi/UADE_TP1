#ifndef COMANDOS_H
#define COMANDOS_H

#include <Arduino.h>
#include "config.h"
#include "tipos.h"
#include "robot.h"


void iniciarComunicacion();

void reportar(const char* mensaje);
void reportar(String mensaje);

void mostrarAyuda();
void leerComandos(Robot &robot);

#endif