#include "planificador.h"

static bool celdasIguales(Celda a, Celda b) {
    return a.x == b.x && a.y == b.y;
}

static bool celdaEsNula(Celda c) {
    return c.x == 9999 && c.y == 9999;
}

bool planificarRuta(
    Celda inicio,
    Celda destino,
    MapaLocal &mapa,
    Ruta &ruta
) {
    ruta.longitud = 0;
    ruta.indiceActual = 0;

    if (!mapa.celdaValida(inicio)) {
        Serial.println("[PLANIFICADOR] Inicio fuera del mapa");
        return false;
    }

    if (!mapa.celdaValida(destino)) {
        Serial.println("[PLANIFICADOR] Destino fuera del mapa");
        return false;
    }

    if (!mapa.celdaTransitable(destino)) {
        Serial.println("[PLANIFICADOR] Destino no transitable");
        return false;
    }

    bool visitado[MAPA_FILAS][MAPA_COLUMNAS];
    Celda padre[MAPA_FILAS][MAPA_COLUMNAS];

    for (int fila = 0; fila < MAPA_FILAS; fila++) {
        for (int col = 0; col < MAPA_COLUMNAS; col++) {
        visitado[fila][col] = false;
        padre[fila][col] = {9999, 9999};
        }
    }

    Celda cola[MAPA_FILAS * MAPA_COLUMNAS];
    int frente = 0;
    int fin = 0;

    int inicioIx = MAPA_ORIGEN_X + inicio.x;
    int inicioIy = MAPA_ORIGEN_Y - inicio.y;

    int destinoIx = MAPA_ORIGEN_X + destino.x;
    int destinoIy = MAPA_ORIGEN_Y - destino.y;

    cola[fin++] = inicio;
    visitado[inicioIy][inicioIx] = true;

    int dx[4] = {0, 1, 0, -1};
    int dy[4] = {1, 0, -1, 0};

    bool encontrado = false;

    while (frente < fin) {
        Celda actual = cola[frente++];

        if (celdasIguales(actual, destino)) {
        encontrado = true;
        break;
        }

        for (int i = 0; i < 4; i++) {
        Celda vecino;
        vecino.x = actual.x + dx[i];
        vecino.y = actual.y + dy[i];

        if (!mapa.celdaValida(vecino)) {
            continue;
        }

        if (!mapa.celdaTransitable(vecino)) {
            continue;
        }

        int vx = MAPA_ORIGEN_X + vecino.x;
        int vy = MAPA_ORIGEN_Y - vecino.y;

        if (visitado[vy][vx]) {
            continue;
        }

        visitado[vy][vx] = true;
        padre[vy][vx] = actual;
        cola[fin++] = vecino;
        }
    }

    if (!encontrado) {
        Serial.println("[PLANIFICADOR] No se encontro ruta");
        return false;
    }

    Celda rutaInvertida[MAPA_FILAS * MAPA_COLUMNAS];
    int longitudInvertida = 0;

    Celda actual = destino;

    while (!celdasIguales(actual, inicio)) {
        if (longitudInvertida >= MAPA_FILAS * MAPA_COLUMNAS) {
        Serial.println("[PLANIFICADOR] Error reconstruyendo ruta");
        return false;
        }

        rutaInvertida[longitudInvertida++] = actual;

        int ax = MAPA_ORIGEN_X + actual.x;
        int ay = MAPA_ORIGEN_Y - actual.y;

        actual = padre[ay][ax];

        if (celdaEsNula(actual)) {
        Serial.println("[PLANIFICADOR] Padre invalido");
        return false;
        }
    }

    for (int i = longitudInvertida - 1; i >= 0; i--) {
        ruta.celdas[ruta.longitud++] = rutaInvertida[i];
    }

    Serial.print("[PLANIFICADOR] Ruta encontrada. Longitud: ");
    Serial.println(ruta.longitud);

    return true;
}

void imprimirRuta(Ruta &ruta) {
    Serial.println();
    Serial.println("===== RUTA =====");

    Serial.print("Longitud: ");
    Serial.println(ruta.longitud);

    Serial.print("Indice actual: ");
    Serial.println(ruta.indiceActual);

    for (int i = 0; i < ruta.longitud; i++) {
        Serial.print(i);
        Serial.print(": (");
        Serial.print(ruta.celdas[i].x);
        Serial.print(", ");
        Serial.print(ruta.celdas[i].y);
        Serial.println(")");
    }

    Serial.println("================");
    Serial.println();
}