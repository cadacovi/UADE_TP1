#ifndef ROBOT_H
#define ROBOT_H

#include <Arduino.h>
#include "config.h"
#include "tipos.h"
#include "mapa_local.h"

class Robot {
    private:
        Celda posicionActual;
        Celda destino;

        Direccion direccionActual;
        EstadoRobot estadoActual;

        MapaLocal mapaLocal;

        bool cargaEntregada;
        bool comandoDescargaManual;
        bool misionActiva;

        Ruta rutaActual;

        unsigned long tiempoEntradaEstado;

        void cambiarEstado(EstadoRobot nuevoEstado);

        void manejarIdle();
        void manejarPlanificandoRuta();
        void manejarNavegando();
        void manejarEvitandoObstaculo();
        void manejarDescargando();
        void manejarMisionCompletada();
        void manejarError();

        Direccion direccionHaciaCelda(Celda siguiente);
        bool orientarHacia(Direccion direccionObjetivo);
        void actualizarDireccionDerecha();
        void actualizarDireccionIzquierda();
        void actualizarDireccion180();

        bool avanzarASiguienteCelda(Celda siguiente);
        Direccion direccionEntreCeldas(Celda desde, Celda hacia);
        int contarCeldasRectasConsecutivas(Direccion direccionTramo);
        void avanzarPosicionLogica(Direccion direccion, int cantidadCeldas);
        bool avanzarDerecho(Celda primeraCelda);

        Celda obtenerCeldaFrontal();
        Celda obtenerCeldaIzquierda();
        Celda obtenerCeldaDerecha();

        void actualizarMapaLocalConSensores();

        const char* estadoComoTexto(EstadoRobot estado);
        const char* direccionComoTexto(Direccion direccion);

    public:
        Robot();

        void inicializar();
        void actualizar();

        void crearNuevaMision(Celda nuevoDestino);
        void detener();
        void solicitarDescargaManual();

        EstadoRobot obtenerEstadoActual();
        Celda obtenerPosicionActual();
        Celda obtenerDestino();
        Direccion obtenerDireccionActual();

        void actualizarMapaSensoresManual();

        void imprimirMapa();

        void imprimirRutaActual();

        void imprimirEstado();
};

#endif