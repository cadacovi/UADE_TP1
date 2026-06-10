## Módulo `ultrasonidos`

### Descripción general

El módulo `ultrasonidos` se encarga de gestionar los sensores HC-SR04 utilizados para detectar obstáculos alrededor del robot. En el proyecto se emplean tres sensores ultrasónicos: uno frontal, uno izquierdo y uno derecho.

Su función principal es medir distancias en milímetros y determinar si existe un obstáculo dentro de ciertos umbrales definidos en `config.h`. Esta información se utiliza para actualizar el mapa local, evitar colisiones y decidir cuándo el robot debe detenerse o replanificar una ruta.

### Funcionamiento general

Los sensores HC-SR04 funcionan emitiendo un pulso ultrasónico mediante el pin `TRIG` y midiendo cuánto tarda en regresar el eco mediante el pin `ECHO`.

El procedimiento básico es:

1. Colocar `TRIG` en bajo durante unos microsegundos.
2. Enviar un pulso alto de 10 microsegundos por `TRIG`.
3. Medir cuánto tiempo permanece alto el pin `ECHO`.
4. Convertir ese tiempo a distancia usando la velocidad del sonido.
5. Dividir por dos, porque el sonido recorre ida y vuelta.

La distancia se calcula mediante:

```cpp
float distanciaMm = (duracion * 0.343) / 2.0;
```

Donde `duracion` está en microsegundos y `0.343` corresponde aproximadamente a la velocidad del sonido en mm/µs.

### Funciones principales

#### `inicializarUltrasonidos()`

Configura los pines `TRIG` como salidas y los pines `ECHO` como entradas para los tres sensores:

* Sensor frontal.
* Sensor izquierdo.
* Sensor derecho.

Además, inicializa los pines `TRIG` en estado bajo para evitar disparos no deseados al comenzar el programa.

#### `medirDistanciaMm(int trigPin, int echoPin)`

Es una función interna que realiza una medición individual de distancia con un sensor ultrasónico específico.

La función envía el pulso de disparo por `TRIG`, mide la duración del pulso recibido en `ECHO` mediante `pulseIn()` y convierte ese tiempo a milímetros.

Si no se recibe eco, o si la distancia medida está fuera del rango considerado válido, la función devuelve `DIST_ULTRASONICO_MAX_MM`. Esto permite tratar lecturas inválidas como ausencia de obstáculo cercano.

Esta función está declarada como `static`, por lo que solo puede utilizarse dentro del archivo `ultrasonidos.cpp`.

#### `medirDistanciaFiltradaMm(int trigPin, int echoPin)`

Realiza varias mediciones consecutivas del mismo sensor y aplica un filtrado por mediana.

Primero toma `ULTRASONICO_MUESTRAS` mediciones, luego las ordena de menor a mayor y devuelve el valor central.

Este filtrado ayuda a reducir el efecto de lecturas erráticas, falsas detecciones o mediciones aisladas incorrectas, que son relativamente comunes en sensores ultrasónicos.

#### `medirDistanciaFiltradaRapidaMm(int trigPin, int echoPin)`

Realiza una versión más rápida del filtrado, usando solo tres muestras y menor retardo entre mediciones.

Se utiliza cuando se necesita una lectura frontal durante el movimiento sin ralentizar demasiado el control del robot. Al usar menos muestras, es menos robusta que la lectura filtrada normal, pero responde más rápido.

#### `leerFrontalMm()`

Devuelve una lectura directa, sin filtrado por mediana, del sensor ultrasónico frontal.

#### `leerIzquierdoMm()`

Devuelve una lectura directa, sin filtrado por mediana, del sensor ultrasónico izquierdo.

#### `leerDerechoMm()`

Devuelve una lectura directa, sin filtrado por mediana, del sensor ultrasónico derecho.

#### `leerFrontalFiltradoMm()`

Devuelve una lectura filtrada por mediana del sensor frontal.

Esta lectura es más estable que una medición individual y se utiliza cuando se prioriza confiabilidad sobre velocidad.

#### `leerFrontalFiltradoRapidoMm()`

Devuelve una lectura frontal filtrada rápidamente usando tres muestras.

Esta función es especialmente útil durante el avance del robot, ya que permite detectar obstáculos frontales sin introducir una demora tan grande como la lectura filtrada completa.

#### `leerIzquierdoFiltradoMm()`

Devuelve una lectura filtrada por mediana del sensor izquierdo.

#### `leerDerechoFiltradoMm()`

Devuelve una lectura filtrada por mediana del sensor derecho.

#### `hayObstaculoFrontal()`

Devuelve `true` si la distancia frontal filtrada es menor o igual que `DIST_OBSTACULO_FRENTE_MM`.

Esta función permite decidir si hay un obstáculo delante del robot.

#### `hayObstaculoIzquierdo()`

Devuelve `true` si la distancia izquierda filtrada es menor o igual que `DIST_OBSTACULO_LATERAL_MM`.

#### `hayObstaculoDerecho()`

Devuelve `true` si la distancia derecha filtrada es menor o igual que `DIST_OBSTACULO_LATERAL_MM`.

#### `imprimirUltrasonidos()`

Lee las distancias filtradas de los tres sensores e imprime por monitor serie:

* Distancia frontal.
* Distancia izquierda.
* Distancia derecha.
* Indicación de obstáculo frontal.
* Indicación de obstáculo izquierdo.
* Indicación de obstáculo derecho.

Esta función se utiliza para pruebas, diagnóstico y calibración de umbrales.

### Funciones auxiliares internas

#### `ordenar(float datos[], int n)`

Ordena un arreglo de mediciones de menor a mayor mediante un algoritmo simple de burbujeo.

Se utiliza para poder obtener la mediana de las muestras tomadas por los sensores ultrasónicos.

#### `medirDistanciaMm(...)`

Además de ser una función principal de medición, también actúa como base para todas las lecturas filtradas.

### Constantes relacionadas

Este módulo utiliza varias constantes definidas en `config.h`:

* `TRIG_FRONT` y `ECHO_FRONT`: pines del sensor frontal.
* `TRIG_LEFT` y `ECHO_LEFT`: pines del sensor izquierdo.
* `TRIG_RIGHT` y `ECHO_RIGHT`: pines del sensor derecho.
* `DIST_OBSTACULO_FRENTE_MM`: umbral para considerar un obstáculo frontal.
* `DIST_OBSTACULO_LATERAL_MM`: umbral para considerar obstáculos laterales.
* `DIST_ULTRASONICO_MAX_MM`: distancia máxima considerada válida.
* `ULTRASONICO_MUESTRAS`: cantidad de muestras usadas para el filtrado por mediana.

### Relación con otros módulos

El módulo `ultrasonidos` es utilizado principalmente por `robot` y `control_movimiento`.

Durante una misión, las lecturas ultrasónicas permiten actualizar el mapa local marcando celdas como libres u ocupadas. Además, durante el avance, el sensor frontal permite detectar obstáculos imprevistos y detener el movimiento antes de una colisión.

También puede ser utilizado por `comandos` o `wifi_comandos` para mostrar lecturas de sensores en el monitor serie o en la interfaz web.

### Comentarios de diseño

Una decisión importante fue implementar dos tipos de lectura filtrada. La lectura filtrada normal toma más muestras y es más robusta, por lo que resulta adecuada para actualizar el mapa o verificar el entorno. En cambio, la lectura filtrada rápida toma menos muestras y usa menor retardo, por lo que es más conveniente durante el movimiento, cuando el robot necesita reaccionar sin detener demasiado el control.

El uso de la mediana, en lugar del promedio, ayuda a rechazar mediciones aisladas incorrectas. Por ejemplo, si una de las muestras resulta demasiado grande o demasiado pequeña por un error momentáneo del sensor, la mediana tiende a conservar un valor más representativo.

También se incorporó un valor máximo de distancia para tratar lecturas inválidas o ausencia de eco como “sin obstáculo cercano”. Esto evita que una medición fallida sea interpretada directamente como un obstáculo.
