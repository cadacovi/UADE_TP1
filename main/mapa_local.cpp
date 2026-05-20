#include "mapa_local.h"

MapaLocal::MapaLocal() {
    inicializar();
}

void MapaLocal::inicializar() {
    for (int fila = 0; fila < MAPA_FILAS; fila++) {
        for (int columna = 0; columna < MAPA_COLUMNAS; columna++) {
        celdas[fila][columna] = DESCONOCIDA;
        }
    }
}

int MapaLocal::localAIndiceX(int xLocal) {
    return MAPA_ORIGEN_X + xLocal;
}

int MapaLocal::localAIndiceY(int yLocal) {
    return MAPA_ORIGEN_Y - yLocal;
}

bool MapaLocal::celdaValida(Celda celda) {
    int ix = localAIndiceX(celda.x);
    int iy = localAIndiceY(celda.y);

    return ix >= 0 && ix < MAPA_COLUMNAS &&
            iy >= 0 && iy < MAPA_FILAS;
}

void MapaLocal::marcarCelda(Celda celda, EstadoCelda estado) {
    if (!celdaValida(celda)) {
        return;
    }

    int ix = localAIndiceX(celda.x);
    int iy = localAIndiceY(celda.y);

    celdas[iy][ix] = estado;
}

EstadoCelda MapaLocal::obtenerEstadoCelda(Celda celda) {
    if (!celdaValida(celda)) {
        return OCUPADA;
    }

    int ix = localAIndiceX(celda.x);
    int iy = localAIndiceY(celda.y);

    return celdas[iy][ix];
}

bool MapaLocal::celdaTransitable(Celda celda) {
    if (!celdaValida(celda)) {
        return false;
    }

    EstadoCelda estado = obtenerEstadoCelda(celda);

    return estado == DESCONOCIDA ||
            estado == LIBRE ||
            estado == VISITADA;
}

char MapaLocal::simboloCelda(EstadoCelda estado) {
    switch (estado) {
        case DESCONOCIDA: return '?';
        case LIBRE: return '.';
        case OCUPADA: return '#';
        case VISITADA: return 'v';
        default: return '?';
    }
}

void MapaLocal::imprimir(Celda posicionRobot, Celda destino) {
    Serial.println();
    Serial.println("===== MAPA LOCAL =====");

    for (int fila = 0; fila < MAPA_FILAS; fila++) {
        for (int columna = 0; columna < MAPA_COLUMNAS; columna++) {
        int xLocal = columna - MAPA_ORIGEN_X;
        int yLocal = MAPA_ORIGEN_Y - fila;

        Celda actual = {xLocal, yLocal};

        if (actual.x == posicionRobot.x && actual.y == posicionRobot.y) {
            Serial.print("R ");
        }
        else if (actual.x == destino.x && actual.y == destino.y) {
            Serial.print("D ");
        }
        else {
            Serial.print(simboloCelda(celdas[fila][columna]));
            Serial.print(" ");
        }
        }

        Serial.println();
    }

    Serial.println("======================");
    Serial.println();
}