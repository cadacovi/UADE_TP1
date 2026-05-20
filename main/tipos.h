#ifndef TIPOS_H
#define TIPOS_H

#include "config.h"

// =====================================================
// ENUMS
// =====================================================

enum EstadoRobot {
    IDLE,
    PLANIFICANDO_RUTA,
    NAVEGANDO,
    EVITANDO_OBSTACULO,
    DESCARGANDO,
    MISION_COMPLETADA,
    ESTADO_ERROR
};

enum Direccion {
    NORTE,
    ESTE,
    SUR,
    OESTE
};

enum EstadoCelda {
    DESCONOCIDA,
    LIBRE,
    OCUPADA,
    VISITADA
};

// =====================================================
// STRUCTS
// =====================================================

struct Celda {
    int x;
    int y;
};

struct Ruta {
    Celda celdas[MAPA_FILAS * MAPA_COLUMNAS];
    int longitud;
    int indiceActual;
};

#endif