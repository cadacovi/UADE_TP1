#include "robot.h"
#include "comandos.h"
#include "motores.h"
#include "encoders.h"
#include "imu.h"
#include "ultrasonidos.h"
#include "servo_carga.h"
#include "planificador.h"
#include "control_movimiento.h"

Robot::Robot() {
    posicionActual = {0, 0};
    destino = {0, 0};

    direccionActual = NORTE;
    estadoActual = IDLE;

    cargaEntregada = false;
    comandoDescargaManual = false;
    misionActiva = false;

    rutaActual.longitud = 0;
    rutaActual.indiceActual = 0;

    tiempoEntradaEstado = 0;
}

void Robot::inicializar() {
    inicializarMotores();
    inicializarEncoders();
    inicializarIMU();
    inicializarUltrasonidos();
    inicializarServoCarga();

    posicionActual = {0, 0};
    destino = {0, 0};

    mapaLocal.inicializar();
    mapaLocal.marcarCelda(posicionActual, VISITADA);

    direccionActual = NORTE;
    estadoActual = IDLE;

    cargaEntregada = false;
    comandoDescargaManual = false;
    misionActiva = false;

    rutaActual.longitud = 0;
    rutaActual.indiceActual = 0;

    tiempoEntradaEstado = millis();

    reportar("Robot inicializado");
    reportar("Posicion local inicial: (0, 0)");
    reportar("Direccion inicial: NORTE local");
}

void Robot::actualizar() {
    actualizarIMU();

    if (comandoDescargaManual) {
        comandoDescargaManual = false;
        cambiarEstado(DESCARGANDO);
    }

    switch (estadoActual) {
        case IDLE:
        manejarIdle();
        break;

        case PLANIFICANDO_RUTA:
        manejarPlanificandoRuta();
        break;

        case NAVEGANDO:
        manejarNavegando();
        break;

        case EVITANDO_OBSTACULO:
        manejarEvitandoObstaculo();
        break;

        case DESCARGANDO:
        manejarDescargando();
        break;

        case MISION_COMPLETADA:
        manejarMisionCompletada();
        break;

        case ESTADO_ERROR:
        manejarError();
        break;
    }
}

void Robot::cambiarEstado(EstadoRobot nuevoEstado) {
    if (estadoActual == nuevoEstado) {
        return;
    }

    Serial.print("[ESTADO] ");
    Serial.print(estadoComoTexto(estadoActual));
    Serial.print(" -> ");
    Serial.println(estadoComoTexto(nuevoEstado));

    estadoActual = nuevoEstado;
    tiempoEntradaEstado = millis();
}

void Robot::crearNuevaMision(Celda nuevoDestino) {
    detenerMotores();

    posicionActual = {0, 0};
    destino = nuevoDestino;

    direccionActual = NORTE;

    cargaEntregada = false;
    comandoDescargaManual = false;
    misionActiva = true;

    rutaActual.longitud = 0;
    rutaActual.indiceActual = 0;

    mapaLocal.inicializar();
    mapaLocal.marcarCelda(posicionActual, VISITADA);

    reportar("Nueva mision creada");

    Serial.print("Destino local: x=");
    Serial.print(destino.x);
    Serial.print(" y=");
    Serial.println(destino.y);

    cambiarEstado(PLANIFICANDO_RUTA);
}

void Robot::detener() {
    detenerMotores();

    misionActiva = false;
    comandoDescargaManual = false;

    reportar("Robot detenido por comando");

    cambiarEstado(IDLE);
}

void Robot::solicitarDescargaManual() {
    comandoDescargaManual = true;
}

EstadoRobot Robot::obtenerEstadoActual() {
    return estadoActual;
}

Celda Robot::obtenerPosicionActual() {
    return posicionActual;
}

Celda Robot::obtenerDestino() {
    return destino;
}

Direccion Robot::obtenerDireccionActual() {
    return direccionActual;
}

void Robot::imprimirEstado() {
    Serial.println();
    Serial.println("===== ESTADO ROBOT =====");

    Serial.print("Estado actual: ");
    Serial.println(estadoComoTexto(estadoActual));

    Serial.print("Posicion local: x=");
    Serial.print(posicionActual.x);
    Serial.print(" y=");
    Serial.println(posicionActual.y);

    Serial.print("Destino local: x=");
    Serial.print(destino.x);
    Serial.print(" y=");
    Serial.println(destino.y);

    Serial.print("Direccion: ");
    Serial.println(direccionComoTexto(direccionActual));

    Serial.print("Mision activa: ");
    Serial.println(misionActiva ? "SI" : "NO");

    Serial.print("Carga entregada: ");
    Serial.println(cargaEntregada ? "SI" : "NO");

    Serial.print("Ruta longitud: ");
    Serial.println(rutaActual.longitud);

    Serial.print("Ruta indice actual: ");
    Serial.println(rutaActual.indiceActual);

    Serial.println("========================");
    Serial.println();
}

void Robot::imprimirMapa() {
    mapaLocal.imprimir(posicionActual, destino);
}

void Robot::imprimirRutaActual() {
    imprimirRuta(rutaActual);
}

// =====================================================
// ACTUALIZAR MAPA LOCAL
// =====================================================

Celda Robot::obtenerCeldaFrontal() {
    Celda celda = posicionActual;

    switch (direccionActual) {
        case NORTE:
        celda.y += 1;
        break;

        case ESTE:
        celda.x += 1;
        break;

        case SUR:
        celda.y -= 1;
        break;

        case OESTE:
        celda.x -= 1;
        break;
    }

    return celda;
}

Celda Robot::obtenerCeldaIzquierda() {
    Celda celda = posicionActual;

    switch (direccionActual) {
        case NORTE:
        celda.x -= 1;
        break;

        case ESTE:
        celda.y += 1;
        break;

        case SUR:
        celda.x += 1;
        break;

        case OESTE:
        celda.y -= 1;
        break;
    }

    return celda;
}

Celda Robot::obtenerCeldaDerecha() {
    Celda celda = posicionActual;

    switch (direccionActual) {
        case NORTE:
        celda.x += 1;
        break;

        case ESTE:
        celda.y -= 1;
        break;

        case SUR:
        celda.x -= 1;
        break;

        case OESTE:
        celda.y += 1;
        break;
    }

    return celda;
}

void Robot::actualizarMapaLocalConSensores() {
    float distanciaFrontal = leerFrontalFiltradoMm();
    float distanciaIzquierda = leerIzquierdoFiltradoMm();
    float distanciaDerecha = leerDerechoFiltradoMm();

    Celda frontal = obtenerCeldaFrontal();
    Celda izquierda = obtenerCeldaIzquierda();
    Celda derecha = obtenerCeldaDerecha();

    if (distanciaFrontal <= DIST_OBSTACULO_FRENTE_MM) {
        mapaLocal.marcarCelda(frontal, OCUPADA);
    } else {
        if (mapaLocal.obtenerEstadoCelda(frontal) == DESCONOCIDA) {
        mapaLocal.marcarCelda(frontal, LIBRE);
        }
    }

    if (distanciaIzquierda <= DIST_OBSTACULO_LATERAL_MM) {
        mapaLocal.marcarCelda(izquierda, OCUPADA);
    } else {
        if (mapaLocal.obtenerEstadoCelda(izquierda) == DESCONOCIDA) {
        mapaLocal.marcarCelda(izquierda, LIBRE);
        }
    }

    if (distanciaDerecha <= DIST_OBSTACULO_LATERAL_MM) {
        mapaLocal.marcarCelda(derecha, OCUPADA);
    } else {
        if (mapaLocal.obtenerEstadoCelda(derecha) == DESCONOCIDA) {
        mapaLocal.marcarCelda(derecha, LIBRE);
        }
    }

    mapaLocal.marcarCelda(posicionActual, VISITADA);

    if (DEBUG_MAPA || DEBUG_SENSORES) {
        Serial.println("[MAPA] Actualizado con sensores");

        Serial.print("Frontal mm: ");
        Serial.println(distanciaFrontal);

        Serial.print("Izquierdo mm: ");
        Serial.println(distanciaIzquierda);

        Serial.print("Derecho mm: ");
        Serial.println(distanciaDerecha);
    }
}

void Robot::actualizarMapaSensoresManual() {
    actualizarMapaLocalConSensores();
    mapaLocal.imprimir(posicionActual, destino);
}

// =====================================================
// NAVEGACION
// =====================================================
Direccion Robot::direccionHaciaCelda(Celda siguiente) {
    if (siguiente.x > posicionActual.x) {
        return ESTE;
    }

    if (siguiente.x < posicionActual.x) {
        return OESTE;
    }

    if (siguiente.y > posicionActual.y) {
        return NORTE;
    }

    return SUR;
}

void Robot::actualizarDireccionDerecha() {
    direccionActual = (Direccion)((direccionActual + 1) % 4);
}

void Robot::actualizarDireccionIzquierda() {
    direccionActual = (Direccion)((direccionActual + 3) % 4);
}

void Robot::actualizarDireccion180() {
    direccionActual = (Direccion)((direccionActual + 2) % 4);
}

bool Robot::orientarHacia(Direccion direccionObjetivo) {
    while (direccionActual != direccionObjetivo) {
        int diferencia = (direccionObjetivo - direccionActual + 4) % 4;

        bool giroOk = false;

        if (diferencia == 1) {
        giroOk = girar90Derecha();

        if (giroOk) {
            actualizarDireccionDerecha();
        }
        }

        else if (diferencia == 3) {
        giroOk = girar90Izquierda();

        if (giroOk) {
            actualizarDireccionIzquierda();
        }
        }

        else if (diferencia == 2) {
        giroOk = girar180();

        if (giroOk) {
            actualizarDireccion180();
        }
        }

        if (!giroOk) {
        reportar("Error al orientar robot");
        return false;
        }

        delay(100);
    }

    return true;
}

bool Robot::avanzarASiguienteCelda(Celda siguiente) {
    Direccion direccionNecesaria = direccionHaciaCelda(siguiente);

    bool orientacionOk = orientarHacia(direccionNecesaria);

    if (!orientacionOk) {
        cambiarEstado(ESTADO_ERROR);
        return false;
    }

    actualizarMapaLocalConSensores();

    if (mapaLocal.obtenerEstadoCelda(siguiente) == OCUPADA) {
        reportar("Siguiente celda ocupada antes de avanzar");
        cambiarEstado(PLANIFICANDO_RUTA);
        return false;
    }

    bool avanceOk = avanzarUnaCelda();

    if (avanceOk) {
        posicionActual = siguiente;
        mapaLocal.marcarCelda(posicionActual, VISITADA);

        Serial.print("[NAV] Nueva posicion: x=");
        Serial.print(posicionActual.x);
        Serial.print(" y=");
        Serial.println(posicionActual.y);

        return true;
    }

    else {
        reportar("Avance fallido. Posible obstaculo frontal");

        Celda frontal = obtenerCeldaFrontal();
        mapaLocal.marcarCelda(frontal, OCUPADA);

        cambiarEstado(EVITANDO_OBSTACULO);
        return false;
    }
}

// =====================================================
// MANEJADORES DE ESTADO
// =====================================================

void Robot::manejarIdle() {
    detenerMotores();
    // Espera comandos desde leerComandos().
}

void Robot::manejarPlanificandoRuta() {
    if (!misionActiva) {
        cambiarEstado(IDLE);
        return;
    }

    actualizarMapaLocalConSensores();

    bool rutaOk = planificarRuta(
        posicionActual,
        destino,
        mapaLocal,
        rutaActual
    );

    if (rutaOk) {
        imprimirRuta(rutaActual);
        cambiarEstado(NAVEGANDO);
    } else {
        reportar("No se pudo planificar ruta");
        cambiarEstado(ESTADO_ERROR);
    }
}

void Robot::manejarNavegando() {
    actualizarMapaLocalConSensores();

    if (posicionActual.x == destino.x && posicionActual.y == destino.y) {
        reportar("Destino alcanzado");
        cambiarEstado(DESCARGANDO);
        return;
    }

    if (rutaActual.indiceActual >= rutaActual.longitud) {
        reportar("Ruta terminada, pero destino no alcanzado. Replanificando");
        cambiarEstado(PLANIFICANDO_RUTA);
        return;
    }

    Celda siguiente = rutaActual.celdas[rutaActual.indiceActual];

    if (!mapaLocal.celdaTransitable(siguiente)) {
        reportar("Siguiente celda no transitable. Replanificando");
        cambiarEstado(PLANIFICANDO_RUTA);
        return;
    }

    Serial.print("[NAV] Siguiente celda: x=");
    Serial.print(siguiente.x);
    Serial.print(" y=");
    Serial.println(siguiente.y);

    bool avanceOk = avanzarASiguienteCelda(siguiente);

    if (!avanceOk) {
        return;
    }

    rutaActual.indiceActual++;

    if (posicionActual.x == destino.x && posicionActual.y == destino.y) {
        reportar("Destino alcanzado");
        cambiarEstado(DESCARGANDO);
        return;
    }
}

void Robot::manejarEvitandoObstaculo() {
    detenerMotores();

    reportar("Manejando obstaculo");

    Celda frontal = obtenerCeldaFrontal();
    mapaLocal.marcarCelda(frontal, OCUPADA);

    actualizarMapaLocalConSensores();

    cambiarEstado(PLANIFICANDO_RUTA);
}

void Robot::manejarDescargando() {
    detenerMotores();

    descargarCarga();

    cargaEntregada = true;

    cambiarEstado(MISION_COMPLETADA);
}

void Robot::manejarMisionCompletada() {
    detenerMotores();

    reportar("Mision completada");

    misionActiva = false;

    cambiarEstado(IDLE);
}

void Robot::manejarError() {
    detenerMotores();

    misionActiva = false;

    // El robot queda en error hasta que el usuario mande:
    // RESET, S, o una nueva mision G / GC.
}

// =====================================================
// TEXTO AUXILIAR
// =====================================================

const char* Robot::estadoComoTexto(EstadoRobot estado) {
    switch (estado) {
        case IDLE: return "IDLE";
        case PLANIFICANDO_RUTA: return "PLANIFICANDO_RUTA";
        case NAVEGANDO: return "NAVEGANDO";
        case EVITANDO_OBSTACULO: return "EVITANDO_OBSTACULO";
        case DESCARGANDO: return "DESCARGANDO";
        case MISION_COMPLETADA: return "MISION_COMPLETADA";
        case ESTADO_ERROR: return "ERROR";
        default: return "DESCONOCIDO";
    }
}

const char* Robot::direccionComoTexto(Direccion direccion) {
    switch (direccion) {
        case NORTE: return "NORTE";
        case ESTE: return "ESTE";
        case SUR: return "SUR";
        case OESTE: return "OESTE";
        default: return "DESCONOCIDA";
    }
}