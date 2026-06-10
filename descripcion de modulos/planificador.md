## Módulo `planificador`

### Descripción general

El módulo `planificador` se encarga de calcular una ruta desde la posición actual del robot hasta una celda destino dentro del mapa local. Para ello utiliza el algoritmo BFS, o búsqueda en anchura, aplicado sobre una grilla de celdas.

Este módulo no controla motores ni sensores directamente. Su responsabilidad es puramente lógica: analizar el mapa disponible y determinar por qué celdas puede desplazarse el robot para llegar al objetivo.

El planificador trabaja junto con el módulo `mapa_local`, que indica qué celdas son válidas y cuáles son transitables.

### Algoritmo utilizado: BFS

El algoritmo utilizado es BFS, sigla de *Breadth-First Search*, o búsqueda en anchura.

BFS explora primero las celdas más cercanas al punto de inicio y luego avanza hacia celdas más lejanas. En un mapa de grilla donde cada movimiento entre celdas tiene el mismo costo, BFS permite encontrar una ruta con la menor cantidad de pasos.

En este proyecto, el robot puede desplazarse a las cuatro celdas vecinas principales:

* Norte.
* Este.
* Sur.
* Oeste.

No se consideran movimientos diagonales.

### Funcionamiento general

La función principal `planificarRuta(...)` realiza los siguientes pasos:

1. Limpia la ruta anterior.
2. Verifica que la celda inicial esté dentro del mapa.
3. Verifica que la celda destino esté dentro del mapa.
4. Verifica que el destino sea transitable.
5. Inicializa una matriz de celdas visitadas.
6. Inicializa una matriz de padres para reconstruir la ruta.
7. Crea una cola para ejecutar BFS.
8. Agrega la celda inicial a la cola.
9. Explora las celdas vecinas en las cuatro direcciones principales.
10. Ignora celdas fuera del mapa, no transitables o ya visitadas.
11. Si encuentra el destino, reconstruye la ruta desde el destino hacia el inicio usando la matriz de padres.
12. Invierte esa ruta para que quede ordenada desde el inicio hacia el destino.
13. Guarda el resultado en la estructura `Ruta`.

### Funciones principales

#### `planificarRuta(Celda inicio, Celda destino, MapaLocal &mapa, Ruta &ruta)`

Es la función principal del módulo.

Recibe:

* `inicio`: celda desde donde parte el robot.
* `destino`: celda objetivo.
* `mapa`: referencia al mapa local actual.
* `ruta`: estructura donde se almacenará la ruta encontrada.

Devuelve `true` si logró encontrar una ruta válida y `false` si no fue posible.

Antes de iniciar la búsqueda, la función reinicia los campos principales de la ruta:

```cpp id="uyxt8o"
ruta.longitud = 0;
ruta.indiceActual = 0;
```

Luego valida que el inicio y el destino estén dentro del mapa, y que el destino sea transitable. Si alguna de estas condiciones no se cumple, la planificación falla.

### Estructuras internas del algoritmo

#### Matriz `visitado`

```cpp id="k2cqi5"
bool visitado[MAPA_FILAS][MAPA_COLUMNAS];
```

Esta matriz indica qué celdas ya fueron exploradas por BFS. Sirve para evitar visitar repetidamente la misma celda y para impedir ciclos infinitos.

#### Matriz `padre`

```cpp id="ga55eb"
Celda padre[MAPA_FILAS][MAPA_COLUMNAS];
```

Esta matriz guarda desde qué celda se llegó a cada celda visitada. Luego se utiliza para reconstruir la ruta final desde el destino hacia el inicio.

Cada celda padre se inicializa con un valor nulo artificial:

```cpp id="47dh6l"
{9999, 9999}
```

Ese valor se usa para detectar errores durante la reconstrucción de ruta.

#### Cola de BFS

```cpp id="2nxbv6"
Celda cola[MAPA_FILAS * MAPA_COLUMNAS];
```

La cola almacena las celdas pendientes por explorar. Se implementa mediante un arreglo y dos índices:

* `frente`: posición desde donde se extrae la próxima celda.
* `fin`: posición donde se agrega una nueva celda.

Este enfoque evita usar estructuras dinámicas y mantiene el código simple para un sistema embebido.

### Exploración de vecinos

El algoritmo usa dos arreglos para representar los desplazamientos hacia las cuatro celdas vecinas:

```cpp id="5l9v9b"
int dx[4] = {0, 1, 0, -1};
int dy[4] = {1, 0, -1, 0};
```

Estos desplazamientos corresponden a:

* `(0, 1)`: norte.
* `(1, 0)`: este.
* `(0, -1)`: sur.
* `(-1, 0)`: oeste.

Para cada celda actual, el planificador genera sus vecinos y verifica:

* Que la celda esté dentro del mapa.
* Que la celda sea transitable.
* Que no haya sido visitada previamente.

Si cumple esas condiciones, se marca como visitada, se guarda su padre y se agrega a la cola.

### Reconstrucción de ruta

Una vez encontrado el destino, la ruta se reconstruye desde el destino hacia el inicio.

Para eso, el algoritmo parte de la celda destino y consulta repetidamente la matriz `padre` hasta llegar a la celda inicial.

Como este proceso genera la ruta en orden inverso, primero se guarda en un arreglo temporal llamado `rutaInvertida`. Luego se copia en orden inverso hacia `ruta.celdas`, quedando finalmente ordenada desde el inicio hacia el destino.

La celda inicial no se incluye dentro de la ruta final, porque el robot ya se encuentra en esa posición. Por eso la ruta contiene únicamente las próximas celdas que debe recorrer.

### Funciones auxiliares internas

#### `celdasIguales(Celda a, Celda b)`

Compara dos celdas y devuelve `true` si sus coordenadas `x` e `y` son iguales.

Se utiliza para saber si BFS llegó al destino o si la reconstrucción de ruta alcanzó el inicio.

#### `celdaEsNula(Celda c)`

Devuelve `true` si una celda tiene el valor especial `{9999, 9999}`.

Se utiliza durante la reconstrucción de ruta para detectar un padre inválido o una falla en el proceso de planificación.

### `imprimirRuta(Ruta &ruta)`

Imprime por monitor serie el contenido de una ruta planificada.

Muestra:

* Longitud de la ruta.
* Índice actual.
* Coordenadas de cada celda de la ruta.

Esta función es útil para depuración, ya que permite verificar qué camino calculó el planificador antes de que el robot lo ejecute físicamente.

### Constantes y tipos relacionados

Este módulo utiliza constantes definidas en `config.h`:

* `MAPA_FILAS`
* `MAPA_COLUMNAS`
* `MAPA_ORIGEN_X`
* `MAPA_ORIGEN_Y`

También utiliza tipos definidos en `tipos.h`:

* `Celda`
* `Ruta`

Y utiliza la clase `MapaLocal`, definida en `mapa_local.h`, para consultar si una celda es válida o transitable.

### Relación con otros módulos

El módulo `planificador` se relaciona principalmente con:

* `mapa_local`: para consultar el estado de las celdas y verificar transitabilidad.
* `robot`: para calcular una ruta desde la posición actual hasta el destino de la misión.
* `tipos`: para usar las estructuras `Celda` y `Ruta`.

Durante la navegación, si el robot detecta un obstáculo nuevo, el módulo `robot` actualiza el mapa marcando la celda como ocupada y luego puede llamar nuevamente a `planificarRuta()` para obtener una ruta alternativa.

### Comentarios de diseño

La elección de BFS es adecuada para este proyecto porque el entorno se representa como una grilla y todos los movimientos entre celdas tienen el mismo costo. En esas condiciones, BFS permite encontrar una ruta de mínima cantidad de celdas hasta el destino.

Además, el uso de arreglos de tamaño fijo evita asignación dinámica de memoria, lo cual es conveniente en sistemas embebidos. El tamaño máximo de las estructuras depende directamente de `MAPA_FILAS * MAPA_COLUMNAS`, por lo que el algoritmo queda acotado al tamaño definido para el mapa local.

El planificador considera transitables las celdas desconocidas, libres y visitadas, según la lógica definida en `MapaLocal`. Esto permite que el robot pueda avanzar hacia zonas aún no exploradas, y si aparece un obstáculo durante la ejecución, el mapa se actualiza y la ruta se recalcula.

En conjunto, este módulo permite que el robot no dependa de una trayectoria fija, sino que pueda adaptar su camino según los obstáculos detectados durante la misión.
