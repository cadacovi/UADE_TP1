#ifndef MAPA_LOCAL_H
#define MAPA_LOCAL_H

#include <Arduino.h>
#include "config.h"
#include "tipos.h"

class MapaLocal {
  private:
    EstadoCelda celdas[MAPA_FILAS][MAPA_COLUMNAS];

    int localAIndiceX(int xLocal);
    int localAIndiceY(int yLocal);

    char simboloCelda(EstadoCelda estado);

  public:
    MapaLocal();

    void inicializar();

    bool celdaValida(Celda celda);

    void marcarCelda(Celda celda, EstadoCelda estado);
    EstadoCelda obtenerEstadoCelda(Celda celda);

    bool celdaTransitable(Celda celda);

    void imprimir(Celda posicionRobot, Celda destino);
};

#endif