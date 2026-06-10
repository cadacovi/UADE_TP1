## Módulo `robot`

### Descripción general

El módulo `robot` es el módulo integrador principal del proyecto. Su función es coordinar el comportamiento completo del robot durante una misión, conectando los módulos de bajo nivel, como motores, encoders, IMU, sensores ultrasónicos y servo, con los módulos de navegación, mapa local y planificación de ruta.

A diferencia de otros módulos más específicos, `robot` no se encarga solamente de un sensor o actuador particular. En cambio, administra el estado general del sistema, la posición lógica del robot, el destino, la ruta actual, la actualización del mapa, la navegación entre celdas, la detección de obstáculos, la descarga de carga y el retorno al origen.

Por este motivo, es el archivo más amplio del proyecto: funciona como una capa de control de alto nivel.

### Variables internas principales

La clase `Robot` mantiene varias variables internas que representan el estado de la misión:

* `posicionActual`: celda lógica donde se encuentra el robot dentro del mapa local.
* `destino`: celda objetivo hacia la cual debe dirigirse el robot.
* `direccionActual`: orientación lógica actual del robot, expresada como `NORTE`, `ESTE`, `SUR` u `OESTE`.
* `estadoActual`: estado actual de la máquina de estados del robot.
* `mapaLocal`: objeto encargado de almacenar el estado de las celdas del entorno.
* `rutaActual`: ruta calculada por el planificador BFS.
* `cargaEntregada`: indica si la carga ya fue descargada.
* `comandoDescargaManual`: bandera usada para solicitar descarga manual.
* `misionActiva`: indica si hay una misión en ejecución.
* `regresandoAlOrigen`: indica si el robot ya entregó la carga y está volviendo a la posición inicial.
* `tiempoEntradaEstado`: almacena el instante en que se ingresó al estado actual.

Estas variables permiten que el robot mantenga memoria de su misión y tome decisiones según el contexto.

### Inicialización del robot

#### `Robot::Robot()`

El constructor inicializa las variables internas en un estado seguro:

* Posición inicial en `(0, 0)`.
* Destino inicial en `(0, 0)`.
* Dirección inicial `NORTE`.
* Estado inicial `IDLE`.
* Misión inactiva.
* Carga no entregada.
* Ruta vacía.

Esta inicialización garantiza que el objeto `Robot` comience en una condición conocida antes de ejecutar cualquier misión.

#### `inicializar()`

La función `inicializar()` realiza la inicialización general del sistema. Llama a los módulos de hardware principales:

* `inicializarMotores()`
* `inicializarEncoders()`
* `inicializarIMU()`
* `inicializarUltrasonidos()`
* `inicializarServoCarga()`

Luego reinicia la posición lógica del robot, el destino, la dirección, las banderas de misión y la ruta actual. También inicializa el mapa local y marca la celda inicial como `VISITADA`.

Esta función deja el robot listo para recibir comandos o iniciar una misión.

### Creación y detención de misiones

#### `crearNuevaMision(Celda nuevoDestino)`

Esta función crea una nueva misión a partir de un destino relativo indicado por el usuario.

Al iniciar una nueva misión:

1. Detiene los motores.
2. Reinicia la posición lógica a `(0, 0)`.
3. Guarda el nuevo destino.
4. Define la dirección inicial como `NORTE`.
5. Reinicia las banderas de carga, descarga manual y retorno al origen.
6. Activa la misión.
7. Limpia la ruta anterior.
8. Reinicia el mapa local.
9. Marca la celda inicial como `VISITADA`.
10. Cambia el estado a `PLANIFICANDO_RUTA`.

De esta manera, cada misión comienza con un mapa local nuevo y con coordenadas relativas al punto de partida del robot.

#### `detener()`

Detiene inmediatamente los motores, desactiva la misión y cancela cualquier solicitud de descarga manual. Luego cambia el estado del robot a `IDLE`.

Esta función es importante para implementar comandos de parada manual o emergencia.

#### `solicitarDescargaManual()`

Activa la bandera `comandoDescargaManual`. En la siguiente actualización del robot, esta bandera provoca el cambio al estado `DESCARGANDO`.

### Máquina de estados

El comportamiento general del robot se implementa mediante una máquina de estados. Cada estado representa una etapa distinta de la misión.

Los estados principales son:

* `IDLE`: robot en reposo, esperando comandos.
* `PLANIFICANDO_RUTA`: cálculo de ruta hacia el destino actual.
* `NAVEGANDO`: ejecución de la ruta planificada.
* `EVITANDO_OBSTACULO`: manejo de obstáculo detectado.
* `DESCARGANDO`: activación del servo de descarga.
* `MISION_COMPLETADA`: finalización de la misión.
* `ESTADO_ERROR`: estado de error ante una falla.

#### `actualizar()`

Es la función que debe ejecutarse periódicamente desde el programa principal. Primero actualiza la IMU y luego revisa si existe una solicitud de descarga manual.

Después, según el valor de `estadoActual`, llama al manejador correspondiente:

* `manejarIdle()`
* `manejarPlanificandoRuta()`
* `manejarNavegando()`
* `manejarEvitandoObstaculo()`
* `manejarDescargando()`
* `manejarMisionCompletada()`
* `manejarError()`

Esta estructura permite separar la lógica de cada estado y ordenar el flujo general del programa.

#### `cambiarEstado(EstadoRobot nuevoEstado)`

Cambia el estado actual del robot. Si el nuevo estado es igual al estado actual, no hace nada.

Cuando ocurre un cambio de estado, imprime la transición por monitor serie y actualiza `tiempoEntradaEstado`. Esto facilita la depuración, ya que permite observar en qué etapa de la misión se encuentra el robot.

### Actualización del mapa local con sensores

El robot utiliza los sensores ultrasónicos para actualizar el mapa local. Para ello primero debe determinar qué celda corresponde al frente, a la izquierda y a la derecha según su orientación actual.

#### `obtenerCeldaFrontal()`

Devuelve la celda ubicada delante del robot según `direccionActual`.

Por ejemplo:

* Si el robot mira al `NORTE`, la celda frontal es `(x, y + 1)`.
* Si mira al `ESTE`, la celda frontal es `(x + 1, y)`.
* Si mira al `SUR`, la celda frontal es `(x, y - 1)`.
* Si mira al `OESTE`, la celda frontal es `(x - 1, y)`.

#### `obtenerCeldaIzquierda()`

Devuelve la celda ubicada a la izquierda del robot, teniendo en cuenta la orientación actual.

#### `obtenerCeldaDerecha()`

Devuelve la celda ubicada a la derecha del robot, también en función de la orientación actual.

#### `actualizarMapaLocalConSensores()`

Lee las distancias frontal, izquierda y derecha mediante los sensores ultrasónicos filtrados.

Luego compara cada distancia contra los umbrales definidos en `config.h`:

* Si la distancia frontal es menor o igual que `DIST_OBSTACULO_FRENTE_MM`, la celda frontal se marca como `OCUPADA`.
* Si la distancia izquierda es menor o igual que `DIST_OBSTACULO_LATERAL_MM`, la celda izquierda se marca como `OCUPADA`.
* Si la distancia derecha es menor o igual que `DIST_OBSTACULO_LATERAL_MM`, la celda derecha se marca como `OCUPADA`.

Si no se detecta obstáculo y la celda todavía estaba `DESCONOCIDA`, se marca como `LIBRE`.

Finalmente, la posición actual del robot se marca como `VISITADA`.

Esta función permite que el mapa local se vaya actualizando durante la misión a partir de la percepción del entorno.

### Navegación y orientación

#### `direccionHaciaCelda(Celda siguiente)`

Determina qué dirección debe tomar el robot para avanzar desde su posición actual hacia una celda vecina.

Si la siguiente celda está a la derecha, devuelve `ESTE`; si está a la izquierda, devuelve `OESTE`; si está adelante en el eje local, devuelve `NORTE`; y si está detrás, devuelve `SUR`.

#### `actualizarDireccionDerecha()`

Actualiza la dirección lógica del robot después de un giro de 90° hacia la derecha.

#### `actualizarDireccionIzquierda()`

Actualiza la dirección lógica del robot después de un giro de 90° hacia la izquierda.

#### `actualizarDireccion180()`

Actualiza la dirección lógica del robot después de un giro de 180°.

#### `orientarHacia(Direccion direccionObjetivo)`

Gira físicamente el robot hasta que su dirección lógica coincida con la dirección objetivo.

Para hacerlo, calcula la diferencia entre la dirección actual y la dirección deseada. Según esa diferencia, ejecuta:

* Giro de 90° a la derecha.
* Giro de 90° a la izquierda.
* Giro de 180°.

Los giros se realizan mediante `girarAnguloResultado()`, que devuelve información sobre el éxito del giro y el error angular final.

Si el giro es exitoso, la función actualiza la dirección lógica del robot. Si el error final es mayor que una tolerancia definida, intenta realizar una corrección fina.

Esta función es importante porque sincroniza el movimiento físico del robot con su representación lógica dentro del mapa.

### Avance hacia celdas y navegación básica

#### `avanzarASiguienteCelda(Celda siguiente)`

Esta función implementa la lógica básica para avanzar hacia una celda específica.

Primero determina la dirección necesaria para llegar a esa celda y orienta el robot hacia ella. Luego actualiza el mapa con sensores y verifica si la celda siguiente está ocupada.

Si la celda está libre o transitable, llama a `avanzarDistanciaResultado(TAM_CELDA_MM)` para avanzar una celda completa.

Si el avance termina correctamente, actualiza la posición lógica del robot y marca la nueva celda como `VISITADA`.

Si durante el avance se detecta un obstáculo, marca la celda frontal como `OCUPADA`, intenta retroceder y cambia al estado `EVITANDO_OBSTACULO`.

Aunque la versión actual utiliza principalmente el avance optimizado por tramos rectos, esta función representa la lógica base de navegación celda a celda.

### Avance optimizado por tramos rectos

Una mejora importante del módulo `robot` es la posibilidad de avanzar varias celdas consecutivas en línea recta sin detenerse en cada celda.

Esto reduce detenciones innecesarias, mejora la fluidez del movimiento y hace que la navegación sea más natural.

#### `direccionEntreCeldas(Celda desde, Celda hacia)`

Determina la dirección necesaria para ir desde una celda hacia otra celda vecina.

Se utiliza para analizar si varias celdas consecutivas de la ruta pertenecen al mismo tramo recto.

#### `contarCeldasRectasConsecutivas(Direccion direccionTramo)`

Cuenta cuántas celdas consecutivas de la ruta actual se encuentran en la misma dirección de avance.

La función recorre la ruta desde `rutaActual.indiceActual` y se detiene si:

* La dirección cambia.
* Alguna celda no es transitable.
* Se alcanza el máximo definido por `MAX_CELDAS_TRAMO_RECTO`.

El resultado es la cantidad de celdas que el robot puede intentar avanzar en un solo tramo recto.

#### `avanzarPosicionLogica(Direccion direccion, int cantidadCeldas)`

Actualiza la posición lógica del robot después de avanzar cierta cantidad de celdas en una dirección.

Por cada celda avanzada, modifica `posicionActual` según la dirección y marca la nueva celda como `VISITADA`.

Esta función no mueve físicamente el robot; solo actualiza la representación lógica después de que el movimiento físico fue ejecutado correctamente.

#### `avanzarDerecho(Celda primeraCelda)`

Es la función principal de avance optimizado.

Primero orienta el robot hacia la primera celda del tramo. Luego actualiza el mapa y verifica que esa celda sea transitable.

Después calcula cuántas celdas consecutivas pueden recorrerse en línea recta y llama a:

```cpp
avanzarDistanciaResultado(cantidadCeldas * TAM_CELDA_MM);
```

Si el avance se completa correctamente, actualiza la posición lógica del robot, incrementa `rutaActual.indiceActual` y vuelve a actualizar el mapa con sensores.

Si se detecta un obstáculo durante el tramo, el robot:

1. Calcula la distancia recorrida.
2. Estima cuántas celdas completas alcanzó a avanzar.
3. Calcula cuánto debe retroceder para volver al centro de una celda segura.
4. Retrocede si corresponde.
5. Actualiza la posición lógica según las celdas completas recorridas.
6. Marca la celda frontal como `OCUPADA`.
7. Cambia al estado `EVITANDO_OBSTACULO`.

Esta estrategia permite manejar obstáculos detectados durante un avance largo sin perder completamente la referencia lógica de la posición.

### Manejadores de estado

#### `manejarIdle()`

Detiene los motores y deja el robot esperando comandos externos.

#### `manejarPlanificandoRuta()`

Si no hay misión activa, vuelve a `IDLE`.

Si hay misión activa, primero actualiza el mapa local con sensores y luego llama a `planificarRuta()` para calcular una ruta desde `posicionActual` hasta `destino`.

Si la ruta se encuentra correctamente, la imprime y cambia el estado a `NAVEGANDO`. Si no se encuentra ruta, cambia a `ESTADO_ERROR`.

#### `manejarNavegando()`

Es el manejador principal durante la ejecución de una ruta.

Primero actualiza el mapa con sensores. Luego verifica si el robot ya llegó al destino.

Si llegó al destino y no estaba regresando al origen, cambia a `DESCARGANDO`. Si llegó al origen durante el retorno, cambia a `MISION_COMPLETADA`.

Si todavía no llegó, revisa si quedan celdas en la ruta actual. Si la ruta terminó pero el destino no fue alcanzado, vuelve a planificar.

Luego obtiene la siguiente celda de la ruta y verifica si es transitable. Si no lo es, vuelve a `PLANIFICANDO_RUTA`.

Si la celda es válida, llama a `avanzarDerecho()` para ejecutar el siguiente tramo de navegación.

#### `manejarEvitandoObstaculo()`

Detiene los motores, marca la celda frontal como `OCUPADA`, actualiza el mapa con sensores y cambia a `PLANIFICANDO_RUTA`.

Este estado permite que el robot reaccione ante un obstáculo detectado durante el avance y calcule una nueva ruta evitando la celda ocupada.

#### `manejarDescargando()`

Detiene los motores y ejecuta `descargarCarga()`.

Luego marca la carga como entregada.

Si el robot todavía no estaba regresando al origen, activa `regresandoAlOrigen`, cambia el destino a `(0, 0)` y vuelve a `PLANIFICANDO_RUTA`.

Si ya estaba regresando al origen, cambia a `MISION_COMPLETADA`.

Esta lógica permite cumplir una secuencia completa: ir al destino, descargar y luego volver al punto inicial.

#### `manejarMisionCompletada()`

Detiene los motores, informa que la misión fue completada, desactiva la misión, limpia la bandera de retorno al origen y vuelve al estado `IDLE`.

#### `manejarError()`

Detiene los motores y desactiva la misión.

El robot queda en estado de error hasta que el usuario envíe un comando de reinicio, parada o nueva misión.

### Funciones de impresión y diagnóstico

#### `imprimirEstado()`

Imprime por monitor serie el estado general del robot:

* Estado actual.
* Posición local.
* Destino.
* Dirección.
* Si hay misión activa.
* Si la carga fue entregada.
* Longitud de la ruta.
* Índice actual de la ruta.

#### `imprimirMapa()`

Llama al método de impresión del mapa local, mostrando la posición del robot y el destino.

#### `imprimirRutaActual()`

Imprime la ruta actualmente calculada mediante `imprimirRuta()`.

#### `estadoComoTexto(EstadoRobot estado)`

Convierte un estado del robot en texto legible para depuración.

#### `direccionComoTexto(Direccion direccion)`

Convierte una dirección lógica en texto legible.

### Relación con otros módulos

El módulo `robot` integra prácticamente todos los módulos del sistema:

* Usa `motores` para detener el robot en estados de reposo, error o descarga.
* Usa `encoders` indirectamente mediante `control_movimiento`.
* Usa `imu` para mantener actualizado el yaw relativo.
* Usa `ultrasonidos` para actualizar el mapa local.
* Usa `servo_carga` para ejecutar la descarga.
* Usa `mapa_local` para representar el entorno.
* Usa `planificador` para calcular rutas mediante BFS.
* Usa `control_movimiento` para avanzar y girar de forma controlada.
* Se comunica con `comandos` y `wifi_comandos`, que permiten al usuario iniciar misiones, detener el robot o consultar información.

### Comentarios de diseño

La clase `Robot` funciona como el controlador principal de alto nivel. Su tamaño se debe a que concentra la lógica de misión y navegación, pero no reemplaza a los demás módulos. Cada parte de bajo nivel sigue estando separada: los motores se controlan desde `motores`, la distancia desde `encoders`, la orientación desde `imu`, los obstáculos desde `ultrasonidos`, la descarga desde `servo_carga`, el mapa desde `mapa_local` y la ruta desde `planificador`.

Una decisión importante fue implementar el comportamiento mediante una máquina de estados. Esto permite ordenar la ejecución de la misión en etapas claras: esperar, planificar, navegar, evitar obstáculos, descargar, volver al origen y finalizar.

También se decidió trabajar con coordenadas locales, reiniciando la posición del robot a `(0, 0)` al comenzar cada misión. Esta estrategia simplifica la navegación, ya que todo destino se interpreta de manera relativa al punto de partida.

Otra mejora relevante fue el avance por tramos rectos. En lugar de avanzar siempre celda por celda, el robot puede agrupar varias celdas consecutivas de la ruta en un único movimiento recto. Esto mejora la fluidez, aunque requiere una lógica adicional para corregir la posición si aparece un obstáculo durante el tramo.

Finalmente, el retorno al origen se implementa reutilizando el mismo sistema de planificación. Después de descargar, el robot cambia el destino a `(0, 0)`, activa la bandera `regresandoAlOrigen` y vuelve a planificar una ruta. De esta forma, no se necesita una lógica de retorno completamente separada.
