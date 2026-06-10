## Módulo `comandos`

### Descripción general

El módulo `comandos` se encarga de la comunicación por puerto serie entre el usuario y el robot. Su función principal es leer comandos enviados desde el monitor serie, interpretarlos y ejecutar la acción correspondiente.

Este módulo permite iniciar misiones, detener el robot, solicitar una descarga manual, imprimir información de diagnóstico, probar motores, consultar sensores, reiniciar encoders, imprimir el mapa local y verificar la ruta actual.

En la práctica, `comandos` actúa como una interfaz de prueba, depuración y control manual del robot.

### Funciones principales

#### `iniciarComunicacion()`

Inicializa la comunicación serie mediante:

```cpp id="f4rb9e"
Serial.begin(SERIAL_BAUDRATE);
```

Luego espera brevemente, imprime un encabezado de inicio y muestra la lista de comandos disponibles llamando a `mostrarAyuda()`.

Esta función se ejecuta al inicio del programa para dejar habilitada la comunicación con el usuario.

#### `reportar(const char* mensaje)` y `reportar(String mensaje)`

Estas funciones imprimen mensajes por monitor serie con el prefijo:

```text id="4stxod"
[ROBOT]
```

Se utilizan para unificar el formato de los mensajes de estado, advertencia o diagnóstico enviados por el robot.

Existen dos versiones sobrecargadas de la función:

* Una recibe cadenas tipo `const char*`.
* Otra recibe objetos `String`.

Esto permite reportar mensajes desde distintas partes del programa sin preocuparse por el tipo exacto del texto.

#### `mostrarAyuda()`

Imprime por monitor serie la lista de comandos disponibles y una breve explicación de cada uno.

Incluye comandos para:

* Mostrar ayuda.
* Imprimir el estado actual.
* Crear una nueva misión.
* Detener el robot.
* Solicitar descarga.
* Reiniciar el estado lógico.
* Probar motores.
* Consultar encoders, IMU y ultrasonidos.
* Abrir o cerrar el servo.
* Avanzar una celda.
* Girar el robot.
* Imprimir mapa y ruta.

También explica el sistema de coordenadas local utilizado por el robot.

#### `extraerDosEnteros(String comando, int &a, int &b)`

Función auxiliar que extrae dos valores enteros desde un comando recibido por serie.

Se utiliza para comandos que requieren coordenadas, como:

```text id="xrn02w"
G x y
GC x y
```

La función busca espacios dentro del texto, separa las partes correspondientes, las convierte a enteros y las devuelve por referencia en las variables `a` y `b`.

Devuelve `true` si pudo extraer correctamente los dos enteros y `false` si el formato del comando es inválido.

#### `procesarComandoG(String comando, Robot &robot)`

Procesa el comando:

```text id="0kw3xp"
G x y
```

Este comando crea una nueva misión usando coordenadas expresadas directamente en celdas locales.

Por ejemplo:

```text id="bzcivr"
G 2 3
```

indica un destino ubicado 2 celdas hacia la derecha y 3 celdas hacia adelante respecto al punto inicial.

La función extrae los dos enteros, construye una variable `Celda destino` y llama a:

```cpp id="pjk3uz"
robot.crearNuevaMision(destino);
```

#### `procesarComandoGC(String comando, Robot &robot)`

Procesa el comando:

```text id="w6mned"
GC x_cm y_cm
```

Este comando permite crear una misión usando coordenadas expresadas en centímetros.

La función convierte los valores recibidos a celdas locales mediante:

```cpp id="82uzkk"
destino.x = (xCm * 10) / TAM_CELDA_MM;
destino.y = (yCm * 10) / TAM_CELDA_MM;
```

Como `TAM_CELDA_MM` está en milímetros, primero multiplica los centímetros por 10 para obtener milímetros. Luego divide por el tamaño de celda.

Después de la conversión, llama a `robot.crearNuevaMision(destino)`.

#### `leerComandos(Robot &robot)`

Es la función principal del módulo.

Primero verifica si hay datos disponibles en el puerto serie. Si no hay datos, retorna sin hacer nada.

Si hay un comando, lo lee hasta encontrar un salto de línea, elimina espacios extra y lo convierte a mayúsculas para evitar problemas por diferencias de escritura.

Luego compara el comando recibido con las opciones disponibles y ejecuta la acción correspondiente.

### Comandos principales de misión

#### `AYUDA`

Muestra la lista de comandos disponibles.

#### `ESTADO`

Llama a `robot.imprimirEstado()` para mostrar el estado general del robot.

#### `G x y`

Crea una nueva misión usando coordenadas en celdas locales.

#### `GC x y`

Crea una nueva misión usando coordenadas en centímetros, convirtiéndolas luego a celdas.

#### `S`

Detiene el robot mediante `robot.detener()`.

#### `D`

Solicita una descarga manual mediante `robot.solicitarDescargaManual()`.

#### `RESET`

Reinicializa el robot llamando a `robot.inicializar()` y reporta que el estado lógico fue reiniciado.

### Comandos de prueba y diagnóstico

El módulo también incluye comandos para probar o consultar los distintos subsistemas:

* `F`: prueba avanzar.
* `B`: prueba retroceder.
* `L`: prueba girar izquierda.
* `R`: prueba girar derecha.
* `E`: imprime información de encoders.
* `ER`: reinicia encoders.
* `IMU`: imprime datos de la IMU.
* `YAW0`: reinicia el yaw relativo.
* `U`: imprime lecturas de ultrasonidos.
* `SERVO`: imprime estado del servo.
* `ABRIR`: abre la carga.
* `CERRAR`: cierra la carga.
* `AVCELDA`: avanza una celda con control.
* `GDER`: gira 90° a la derecha.
* `GIZQ`: gira 90° a la izquierda.
* `G180`: gira 180°.
* `MAPA`: imprime el mapa local.
* `SENSAR`: actualiza el mapa con sensores ultrasónicos.
* `RUTA`: imprime la ruta actual.

Estos comandos fueron útiles durante el desarrollo para probar cada módulo por separado antes de integrar el comportamiento completo del robot.

### Relación con otros módulos

El módulo `comandos` se relaciona con varios componentes del sistema:

* Usa `Robot` para iniciar misiones, detener el robot, solicitar descargas e imprimir estado, mapa o ruta.
* Usa `motores` para ejecutar pruebas simples de movimiento.
* Usa `encoders` para imprimir o reiniciar contadores.
* Usa `imu` para imprimir datos y reiniciar yaw.
* Usa `ultrasonidos` para imprimir lecturas de distancia.
* Usa `servo_carga` para probar apertura, cierre y estado del servo.
* Usa `control_movimiento` para ejecutar movimientos controlados, como avanzar una celda o girar.

### Comentarios de diseño

Este módulo fue importante durante el desarrollo porque permitió probar el sistema de forma incremental. Antes de ejecutar misiones completas, se podían verificar individualmente motores, encoders, IMU, ultrasonidos, servo y control de movimiento.

La función `leerComandos()` centraliza la interpretación de comandos por serie. Esto facilita extender el sistema agregando nuevos comandos sin modificar el resto de los módulos.

Además, los comandos `G` y `GC` permiten usar dos formas de indicar destinos: directamente en celdas o en centímetros. Esto hace que el robot pueda probarse tanto desde una perspectiva lógica del mapa como desde medidas físicas aproximadas.

Aunque en la versión final se agregó una interfaz WiFi, la comunicación serie sigue siendo útil como herramienta de depuración y respaldo.
