## Módulo `mapa_local`

### Descripción general

El módulo `mapa_local` se encarga de representar el entorno de navegación del robot mediante una grilla de celdas. Cada celda del mapa puede tener un estado determinado, como desconocida, libre, ocupada o visitada.

Este módulo permite almacenar la información obtenida durante la misión, por ejemplo, qué celdas fueron recorridas, cuáles se consideran libres y cuáles contienen obstáculos. Esa información luego es utilizada por el planificador para calcular rutas posibles dentro del entorno.

El mapa implementado es local y temporal. Esto significa que se inicializa al comenzar una misión y toma como referencia la posición inicial del robot.

### Estructura general

El mapa se implementa como una matriz bidimensional:

```cpp
EstadoCelda celdas[MAPA_FILAS][MAPA_COLUMNAS];
```

Cada elemento de la matriz representa una celda del entorno. Las dimensiones del mapa están definidas en `config.h` mediante:

* `MAPA_FILAS`
* `MAPA_COLUMNAS`

Además, se define un origen lógico dentro de la matriz:

* `MAPA_ORIGEN_X`
* `MAPA_ORIGEN_Y`

En la versión actual, el mapa tiene un tamaño de 11 x 11 celdas, y el origen lógico se ubica en el centro de la matriz. De esta forma, el robot puede representar posiciones relativas positivas y negativas respecto a su punto de partida.

### Sistema de coordenadas

El robot trabaja con coordenadas locales, donde la posición inicial de cada misión se considera:

```cpp
(0, 0)
```

Sin embargo, internamente la matriz no puede usar índices negativos. Por eso, el módulo convierte las coordenadas locales del robot a índices de matriz.

Por ejemplo, una celda local:

```cpp
Celda celda = {0, 0};
```

se transforma internamente en la posición central del arreglo, definida por `MAPA_ORIGEN_X` y `MAPA_ORIGEN_Y`.

### Funciones principales

#### `MapaLocal::MapaLocal()`

Constructor de la clase `MapaLocal`.

Al crear un objeto de tipo `MapaLocal`, se llama automáticamente a `inicializar()`, dejando todas las celdas del mapa en estado `DESCONOCIDA`.

#### `inicializar()`

Recorre toda la matriz del mapa y asigna a cada celda el estado `DESCONOCIDA`.

Esta función se utiliza para comenzar una misión con un mapa limpio, sin información previa del entorno.

#### `localAIndiceX(int xLocal)`

Convierte una coordenada local `x` en un índice de columna de la matriz.

La conversión se realiza sumando el desplazamiento local al origen del mapa:

```cpp
return MAPA_ORIGEN_X + xLocal;
```

De esta forma, valores locales negativos o positivos pueden representarse dentro de la matriz.

#### `localAIndiceY(int yLocal)`

Convierte una coordenada local `y` en un índice de fila de la matriz.

La conversión se realiza como:

```cpp
return MAPA_ORIGEN_Y - yLocal;
```

Se resta `yLocal` porque, en una matriz, las filas aumentan hacia abajo, mientras que en el sistema lógico del robot el eje `y` positivo representa avanzar hacia adelante.

#### `celdaValida(Celda celda)`

Verifica si una celda local se encuentra dentro de los límites del mapa.

Primero convierte la celda local a índices de matriz y luego comprueba que esos índices estén dentro del rango permitido.

Devuelve `true` si la celda está dentro del mapa y `false` si queda fuera.

Esta función evita acceder a posiciones inválidas del arreglo.

#### `marcarCelda(Celda celda, EstadoCelda estado)`

Permite cambiar el estado de una celda determinada.

Antes de modificar el mapa, verifica si la celda es válida. Si la celda queda fuera de los límites, la función no realiza ningún cambio.

Se utiliza para marcar celdas como:

* `LIBRE`
* `OCUPADA`
* `VISITADA`
* `DESCONOCIDA`

Por ejemplo, cuando el robot detecta un obstáculo, puede marcar la celda correspondiente como `OCUPADA`.

#### `obtenerEstadoCelda(Celda celda)`

Devuelve el estado actual de una celda.

Si la celda solicitada está fuera del mapa, la función devuelve `OCUPADA`. Esta decisión es importante porque evita que el planificador intente generar rutas fuera de los límites del mapa. En la práctica, todo lo que queda fuera del mapa se trata como no transitable.

#### `celdaTransitable(Celda celda)`

Determina si una celda puede ser usada por el robot para planificar una ruta.

Una celda se considera transitable si su estado es:

* `DESCONOCIDA`
* `LIBRE`
* `VISITADA`

En cambio, una celda `OCUPADA` o fuera del mapa no se considera transitable.

Esta función es utilizada por el planificador para decidir qué posiciones puede explorar durante la búsqueda de ruta.

#### `simboloCelda(EstadoCelda estado)`

Función auxiliar interna que convierte el estado de una celda en un carácter para imprimir el mapa por monitor serie.

Los símbolos utilizados son:

* `?`: celda desconocida.
* `.`: celda libre.
* `#`: celda ocupada.
* `v`: celda visitada.

Esta función está declarada en la parte privada de la clase, ya que solo se usa internamente para mostrar el mapa.

#### `imprimir(Celda posicionRobot, Celda destino)`

Imprime una representación del mapa local en el monitor serie.

Durante la impresión, cada celda se muestra con su símbolo correspondiente. Además:

* La posición actual del robot se muestra como `R`.
* El destino se muestra como `D`.

Esta función es útil para depuración, ya que permite visualizar cómo el robot está interpretando el entorno, qué celdas marcó como ocupadas y dónde se encuentran el robot y el destino dentro del mapa.

### Constantes relacionadas

Este módulo utiliza constantes definidas en `config.h`:

* `MAPA_FILAS`: cantidad de filas del mapa.
* `MAPA_COLUMNAS`: cantidad de columnas del mapa.
* `MAPA_ORIGEN_X`: columna correspondiente al origen lógico.
* `MAPA_ORIGEN_Y`: fila correspondiente al origen lógico.

También utiliza tipos definidos en `tipos.h`:

* `Celda`
* `EstadoCelda`
* `DESCONOCIDA`
* `LIBRE`
* `OCUPADA`
* `VISITADA`

### Relación con otros módulos

El módulo `mapa_local` es utilizado principalmente por:

* `robot`: para actualizar el mapa durante la misión, marcar celdas visitadas, libres u ocupadas.
* `planificador`: para consultar qué celdas son transitables y calcular una ruta hacia el destino.
* `comandos` o funciones de depuración: para imprimir el mapa y verificar el estado interno del sistema.

### Comentarios de diseño

Una decisión importante fue representar el entorno como un mapa local centrado en el origen de la misión. Esto permite trabajar con coordenadas relativas al punto de partida del robot, lo cual simplifica la navegación en un entorno de prueba.

También se decidió considerar las celdas `DESCONOCIDA`, `LIBRE` y `VISITADA` como transitables. Esto permite que el robot pueda planificar rutas incluso hacia zonas que todavía no exploró completamente. Si durante el avance detecta un obstáculo, la celda correspondiente se marca como `OCUPADA` y el sistema puede replanificar.

Otra decisión relevante es que las celdas fuera de los límites del mapa se tratan como `OCUPADA`. Esto evita que el planificador genere rutas inválidas o intente salir de la región representada por la matriz.

En conjunto, este módulo proporciona la representación básica del entorno sobre la cual se apoya la navegación del robot.
