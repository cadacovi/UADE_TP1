## Módulo `tipos.h`

### Descripción general

El archivo `tipos.h` define los tipos de datos compartidos por los distintos módulos del robot. Su función es establecer una representación común para los estados del robot, las direcciones de movimiento, los estados de las celdas del mapa, las rutas planificadas y los resultados de movimientos.

Este módulo permite que archivos como `robot`, `mapa_local`, `planificador` y `control_movimiento` trabajen con las mismas estructuras y enumeraciones, evitando duplicación de definiciones y facilitando la integración del sistema.

### Enumeraciones principales

#### `EstadoRobot`

La enumeración `EstadoRobot` representa los posibles estados generales del robot durante una misión:

* `IDLE`: estado de reposo. El robot no está ejecutando una misión.
* `PLANIFICANDO_RUTA`: el robot está calculando una ruta hacia el destino.
* `NAVEGANDO`: el robot está siguiendo una ruta planificada.
* `EVITANDO_OBSTACULO`: el robot detectó un obstáculo y debe actualizar el mapa o replanificar.
* `DESCARGANDO`: el robot llegó al destino y está accionando el servo de descarga.
* `MISION_COMPLETADA`: la misión fue finalizada correctamente.
* `ESTADO_ERROR`: ocurrió una condición de error, como timeout o imposibilidad de encontrar ruta.

Esta enumeración permite implementar la lógica general del robot como una máquina de estados.

#### `Direccion`

La enumeración `Direccion` representa la orientación lógica del robot dentro del mapa local:

* `NORTE`
* `ESTE`
* `SUR`
* `OESTE`

Estas direcciones no representan necesariamente coordenadas absolutas del mundo real, sino direcciones relativas al inicio de la misión. Al comenzar, el robot se considera orientado hacia el `NORTE` local.

#### `EstadoCelda`

La enumeración `EstadoCelda` representa el estado de cada celda del mapa:

* `DESCONOCIDA`: celda todavía no explorada o sin información suficiente.
* `LIBRE`: celda considerada transitable.
* `OCUPADA`: celda marcada como obstáculo.
* `VISITADA`: celda por la que el robot ya pasó.

Esta clasificación permite que el planificador BFS decida por qué celdas puede circular y cuáles debe evitar.

### Estructuras principales

#### `Celda`

La estructura `Celda` representa una posición lógica dentro del mapa mediante dos coordenadas:

* `x`: coordenada horizontal.
* `y`: coordenada vertical.

Se utiliza para representar la posición actual del robot, el destino y cada punto de una ruta.

#### `Ruta`

La estructura `Ruta` almacena una secuencia de celdas que el robot debe recorrer para llegar al destino.

Contiene:

* `celdas`: arreglo de celdas que forman la ruta.
* `longitud`: cantidad de celdas válidas dentro de la ruta.
* `indiceActual`: posición actual dentro de la ruta.

El tamaño máximo del arreglo depende de las dimensiones del mapa local, por lo que puede contener como máximo `MAPA_FILAS * MAPA_COLUMNAS` celdas.

#### `ResultadoAvance`

La estructura `ResultadoAvance` resume el resultado de una orden de avance:

* `exito`: indica si el avance se completó correctamente.
* `timeout`: indica si el movimiento fue interrumpido por superar el tiempo máximo permitido.
* `obstaculoDetectado`: indica si se detectó un obstáculo durante el avance.
* `distanciaRecorridaMm`: distancia estimada recorrida durante el movimiento.

Esta estructura permite que el módulo de control de movimiento informe al módulo principal no solo si el avance terminó, sino también por qué terminó y, mas importante, la distancia exacta recorrida.

#### `ResultadoGiro`

La estructura `ResultadoGiro` resume el resultado de una orden de giro:

* `exito`: indica si el giro se completó correctamente.
* `timeout`: indica si el giro fue interrumpido por superar el tiempo máximo permitido.
* `errorFinalGrados`: error angular restante al finalizar el giro.

Esta información permite evaluar la precisión del giro y detectar fallas en la ejecución del movimiento.

### Importancia dentro del proyecto

`tipos.h` cumple una función importante como archivo común de definiciones. Gracias a este módulo, todos los componentes del programa comparten las mismas representaciones para estados, direcciones, celdas, rutas y resultados de movimiento.

Esto mejora la organización del código y permite que el sistema se construya de forma modular: el planificador trabaja con `Celda` y `Ruta`, el mapa trabaja con `EstadoCelda`, el robot gestiona su lógica con `EstadoRobot`, y el control de movimiento informa sus resultados mediante `ResultadoAvance` y `ResultadoGiro`.
