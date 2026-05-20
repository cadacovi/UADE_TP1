#ifndef PLANIFICADOR_H
#define PLANIFICADOR_H

#include <Arduino.h>
#include "config.h"
#include "tipos.h"
#include "mapa_local.h"

bool planificarRuta(
    Celda inicio,
    Celda destino,
    MapaLocal &mapa,
    Ruta &ruta
);

void imprimirRuta(Ruta &ruta);

#endif