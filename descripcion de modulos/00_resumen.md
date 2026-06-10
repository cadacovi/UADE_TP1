## Resumen de los módulos de programación

El software del robot se organizó en distintos módulos `.h` y `.cpp`, separando la lógica de sensores, actuadores, navegación, planificación, comunicación y control general. Esta organización permite probar cada parte por separado y facilita la integración del sistema completo.

### `config.h`

Este archivo contiene las constantes generales del proyecto. Define los pines del ESP32-S3, el tamaño de celda, dimensiones del mapa, umbrales de obstáculos, parámetros físicos del robot, valores de PWM, ganancias de control (correccion de errores), configuración de la IMU, servo y WiFi.

Es uno de los archivos más importantes para la calibración, ya que desde allí se ajustan valores como `TAM_CELDA_MM`, `PULSOS_POR_MM`, `DIST_OBSTACULO_FRENTE_MM`, `PWM_BASE_AVANCE`, `KP_ENCODER_AVANCE` y `KP_YAW_AVANCE`. Estos valores tienen relación con tamaños físicos, algunos requieren calibración.

### `tipos.h`

Define los tipos de datos compartidos por el proyecto. Incluye enumeraciones como `EstadoRobot`, `Direccion` y `EstadoCelda`, además de estructuras como `Celda`, `Ruta`, `ResultadoAvance` y `ResultadoGiro`.

Este módulo permite que el resto del código use una representación común para la posición del robot, las rutas, los estados de celda y los resultados de movimiento.

### `motores`

El módulo `motores` controla directamente el driver DRV8833. Permite manejar cada motor mediante un valor PWM con signo: positivo para un sentido, negativo para el sentido contrario y cero para detener. El valor PWM debe por lo menos superar la fricción entre las ruedas y el suelo. Depende del estado de las pilas.

Sus funciones principales son `setMotorIzquierdo()`, `setMotorDerecho()`, `setMotores()` y `detenerMotores()`. También incluye funciones de prueba como `pruebaAvanzar()`, `pruebaRetroceder()`, `pruebaGirarIzquierda()` y `pruebaGirarDerecha()`.

La idea principal de este módulo es abstraer el manejo eléctrico del driver, para que otros módulos puedan pedir movimiento sin escribir directamente sobre los pines del DRV8833.

### `encoders`

El módulo `encoders` se encarga de contar los pulsos de los encoders ópticos LM393 conectados a las ruedas. Estos pulsos permiten estimar la distancia recorrida por cada rueda y la distancia promedio del robot.

La lectura se realiza mediante interrupciones usando `attachInterrupt()` en flanco descendente (`FALLING`), que significa que vamos a contar un pulso cuando el encoder pase de HIGH a LOW. Esto permite contar pulsos aunque el programa esté ejecutando otras tareas. Básicamente, le dice al controlador que pause lo que hace para atender al cambio de pulsos del encoder.

Para reducir errores por ruido, rebote o vibraciones, se implementó un filtro temporal dentro de la interrupción mediante `FILTRO_ENCODER_US`, ignorando pulsos demasiado cercanos entre sí. Sin embargo, esto no evita las interrupciones ocacionadas por el ruido. Los efectos de estas interrupciones son minimizados al reducir las instrucciones que el controlador debe ejecutar por interrupción al mínimo.

También se utilizan variables `volatile` (para indicarle al programa que son variables que pueden cambiar a la mitad de una ejecucion del codigo que la esta usando) y secciones críticas para proteger las variables compartidas entre el programa principal y las rutinas de interrupción.

Funciones importantes:

* `inicializarEncoders()`: configura pines e interrupciones.
* `resetEncoders()`: reinicia los contadores.
* `obtenerPulsosIzquierdo()` y `obtenerPulsosDerecho()`: devuelven los pulsos contados.
* `obtenerDistanciaIzquierdaMm()` y `obtenerDistanciaDerechaMm()`: convierten pulsos a milímetros.
* `obtenerDistanciaPromedioMm()`: estima la distancia recorrida por el robot.
* `obtenerErrorRuedasMm()`: calcula la diferencia de avance entre ambas ruedas.

Este módulo fue clave para lograr odometría y control de avance por distancia. Su buen funcionamiento está ligado a una buena calibracion de la constante PULSOS_POR_MM, que puede variar segun la superficie de contacto/estado de ruedas. 

### `imu`

El módulo `imu` gestiona el MPU6050 mediante comunicación I2C. En este proyecto se usa principalmente el giroscopio en el eje Z para estimar el yaw relativo (angulo relativo a refencia interna del IMU) del robot.

Durante la inicialización, el módulo despierta el MPU6050, configura el giroscopio y realiza una calibración para calcular el offset del eje Z. Luego, durante el movimiento, integra la velocidad angular en el tiempo para obtener el ángulo relativo girado.
```cpp
yawRelativo += gyroZ * dt;
```
El valor importante para la conversion es 131. Para el rango de ±250 °/s, el MPU6050 entrega aproximadamente 131 LSB por cada grado por segundo.
```cpp
gyroZ = (rawGyroZ / 131.0) - offsetGyroZ;
// offsetGyroZ es un valor calibrado al inicializarse el robot. Representa el error del IMU, pues mide la variacion que reporta el IMU cuando esta quieto.
```
Funciones importantes:

* `inicializarIMU()`: inicializa el bus I2C y configura el MPU6050.
* `calibrarIMU()`: calcula el offset del giroscopio Z con el robot quieto.
* `actualizarIMU()`: actualiza la velocidad angular y el yaw relativo.
* `resetYaw()`: reinicia el yaw a cero.
* `obtenerYawRelativo()`: devuelve el ángulo relativo acumulado.

La IMU se usa principalmente para controlar giros y corregir desviaciones durante el avance recto.

### `ultrasonidos`

El módulo `ultrasonidos` controla los tres sensores HC-SR04: frontal, izquierdo y derecho. Su función es medir distancias y detectar obstáculos cercanos.

La distancia se obtiene midiendo la duración del pulso `ECHO` y convirtiéndola a milímetros mediante la velocidad del sonido. Para mejorar la estabilidad de las mediciones se usa filtrado por mediana: se toman varias muestras, se ordenan y se elige el valor central.
La distancia se calcula mediante:
```cpp
float distanciaMm = (duracion * 0.343) / 2.0;
// Donde `duracion` está en microsegundos y `0.343` corresponde aproximadamente a la velocidad del sonido en mm/µs.
```
También existe una lectura frontal rápida, con menos muestras, pensada para detectar obstáculos durante el movimiento sin ralentizar demasiado el control.

Funciones importantes:

* `leerFrontalMm()`, `leerIzquierdoMm()`, `leerDerechoMm()`: lecturas directas.
* `leerFrontalFiltradoMm()`, `leerIzquierdoFiltradoMm()`, `leerDerechoFiltradoMm()`: lecturas filtradas por mediana.
* `leerFrontalFiltradoRapidoMm()`: lectura frontal rápida para movimiento.
* `hayObstaculoFrontal()`, `hayObstaculoIzquierdo()`, `hayObstaculoDerecho()`: detección booleana de obstáculos.

Este módulo permite actualizar el mapa local y detener el avance si aparece un obstáculo frontal.

### `servo_carga`

El módulo `servo_carga` controla el servo MG90S utilizado para el mecanismo de descarga. Usa la librería `ESP32Servo` y trabaja con dos posiciones principales: carga cerrada y carga abierta.

Funciones importantes:

* `inicializarServoCarga()`: configura el servo y lo deja cerrado.
* `abrirCarga()`: mueve el servo a la posición abierta.
* `cerrarCarga()`: mueve el servo a la posición cerrada.
* `descargarCarga()`: abre la carga, espera un tiempo definido y vuelve a cerrar.

Este módulo encapsula la lógica del mecanismo de descarga, permitiendo que el módulo principal solo tenga que llamar a `descargarCarga()` cuando llega al destino.

### `control_movimiento`

El módulo `control_movimiento` ejecuta movimientos físicos controlados. Integra motores, encoders, IMU y ultrasonidos para avanzar distancias, retroceder, girar y detectar obstáculos durante el avance.

Funciones importantes:

* `avanzarDistanciaResultado()`: avanza una distancia determinada y devuelve un resultado detallado.
* `avanzarDistanciaMm()`: versión simplificada que devuelve solo éxito o falla.
* `avanzarUnaCelda()`: avanza una celda completa del mapa.
* `girarAnguloResultado()`: gira un ángulo determinado y devuelve el error final.
* `girarAngulo()`: versión simplificada del giro.
* `girar90Derecha()`, `girar90Izquierda()` y `girar180()`: giros predefinidos.

Durante el avance se aplica una corrección proporcional combinando dos errores:

* Error entre ruedas, obtenido con los encoders.
* Error de yaw, obtenido con la IMU.
```cpp
float correccion = (KP_ENCODER_AVANCE * errorRuedas) +
                   (KP_YAW_AVANCE * errorYaw);
// Sobre las constantes de proporcion: Kp convierte un error medido en cualquier unidad en una corrección en las unidades que nos interesan (en este caso PWM), pero su valor no depende solo de las unidades, sino también de la dinámica real del sistema. Por eso suele ser estimado empiricamente.
```

También se reduce el PWM cerca del final del recorrido para evitar sobrepasarse y se verifica periódicamente el sensor frontal para detener el robot ante obstáculos.

Este módulo es el encargado de convertir órdenes como “avanzar 250 mm” o “girar 90°” en movimientos reales corregidos por sensores.

### `mapa_local`

El módulo `mapa_local` representa el entorno como una grilla de celdas. Cada celda puede estar en uno de estos estados:

* `DESCONOCIDA`
* `LIBRE`
* `OCUPADA`
* `VISITADA`

El mapa es local y temporal: se reinicia al comenzar cada misión y toma como origen la posición inicial del robot. Como internamente se usa una matriz, el módulo convierte coordenadas locales, que pueden ser negativas, a índices de arreglo.

Funciones importantes:

* `inicializar()`: reinicia el mapa como desconocido.
* `marcarCelda()`: cambia el estado de una celda.
* `obtenerEstadoCelda()`: consulta el estado de una celda.
* `celdaValida()`: verifica si una celda está dentro del mapa.
* `celdaTransitable()`: indica si una celda puede usarse para planificar.
* `imprimir()`: muestra el mapa por monitor serie.

Este módulo es la base sobre la que trabaja el planificador BFS.

### `planificador`

El módulo `planificador` calcula rutas dentro del mapa local usando BFS, o búsqueda en anchura. Este algoritmo explora primero las celdas más cercanas al inicio y luego las más lejanas.

Como el mapa se modela como una grilla y cada movimiento entre celdas tiene el mismo costo, BFS permite encontrar una ruta válida con la menor cantidad de pasos.

Funciones importantes:

* `planificarRuta()`: calcula una ruta desde la celda inicial hasta el destino.
* `imprimirRuta()`: muestra por monitor serie la ruta calculada.

Durante la planificación se usan:

* Una cola para explorar celdas.
* Una matriz `visitado` para evitar repetir celdas.
* Una matriz `padre` para reconstruir el camino desde el destino hasta el inicio.

El planificador solo considera movimientos en cuatro direcciones: norte, este, sur y oeste. No se utilizan movimientos diagonales.

### `robot`

El módulo `robot` es el controlador principal del sistema. Integra todos los demás módulos y administra la lógica general de la misión.

Este módulo contiene la posición actual, destino, dirección, estado de la misión, mapa local, ruta actual y banderas como `misionActiva`, `cargaEntregada` y `regresandoAlOrigen`.

Funciones y grupos de funciones importantes:

#### Inicialización y misión

* `inicializar()`: inicializa motores, encoders, IMU, ultrasonidos, servo, mapa y estado inicial.
* `crearNuevaMision()`: recibe un destino, reinicia el mapa local y cambia el estado a planificación.
* `detener()`: detiene el robot y vuelve al estado de reposo.
* `solicitarDescargaManual()`: permite pedir descarga desde comandos externos.

#### Máquina de estados

El robot funciona mediante una máquina de estados (según el "estado" actual del robot, ejecuta una lógica distinta.). La función principal es:

* `actualizar()`: se llama continuamente desde el `loop` y ejecuta la lógica correspondiente al estado actual.

Los estados se manejan con funciones específicas:

* `manejarIdle()`: espera comandos.
* `manejarPlanificandoRuta()`: actualiza sensores y calcula una ruta con BFS.
* `manejarNavegando()`: ejecuta la ruta planificada, chequeando por obstáculos.
* `manejarEvitandoObstaculo()`: marca obstáculos en mapa local y vuelve a planificar.
* `manejarDescargando()`: activa el servo de descarga.
* `manejarMisionCompletada()`: finaliza la misión.
* `manejarError()`: detiene el robot ante fallas.

La función `cambiarEstado()` registra las transiciones entre estados y ayuda a depurar el comportamiento del robot.

#### Actualización del mapa con sensores

* `obtenerCeldaFrontal()`
* `obtenerCeldaIzquierda()`
* `obtenerCeldaDerecha()`
* `actualizarMapaLocalConSensores()`

Estas funciones permiten traducir las lecturas de los sensores ultrasónicos a celdas del mapa. Según la orientación actual del robot, la celda frontal, izquierda y derecha cambian. Si un sensor detecta un obstáculo, la celda correspondiente se marca como `OCUPADA`; si no detecta obstáculo y la celda era desconocida, se marca como `LIBRE`.

#### Navegación y orientación

* `direccionHaciaCelda()`: calcula hacia qué dirección debe mirar el robot para avanzar a la siguiente celda.
* `orientarHacia()`: gira físicamente el robot hasta coincidir con la dirección requerida.
* `actualizarDireccionDerecha()`, `actualizarDireccionIzquierda()` y `actualizarDireccion180()`: actualizan la orientación lógica luego de un giro.

Estas funciones conectan la ruta calculada por BFS con los movimientos físicos del robot.

#### Avance optimizado por tramos rectos

Una de las funciones más importantes del módulo es:

* `avanzarDerecho()`

Esta función permite avanzar varias celdas consecutivas en línea recta, en lugar de detenerse en cada celda. Para eso usa:

* `contarCeldasRectasConsecutivas()`: determina cuántas celdas seguidas de la ruta están en la misma dirección.
* `avanzarPosicionLogica()`: actualiza la posición lógica después de un avance exitoso.
* `direccionEntreCeldas()`: calcula la dirección entre dos celdas vecinas.

Si durante el tramo se detecta un obstáculo, el robot estima cuántas celdas completas alcanzó a recorrer, retrocede hasta una posición segura, actualiza la posición lógica, marca la celda frontal como ocupada y pasa al estado `EVITANDO_OBSTACULO`.

Esta lógica es importante porque mejora la fluidez de navegación y reduce detenciones innecesarias.

#### Descarga y retorno al origen

Cuando el robot llega al destino, cambia al estado `DESCARGANDO` y llama a `descargarCarga()`.

Después de descargar, activa la bandera `regresandoAlOrigen`, cambia el destino a `(0, 0)` y vuelve a planificar una ruta. De esta forma, el retorno al origen reutiliza el mismo sistema de mapa, BFS y navegación.

### `comandos`

El módulo `comandos` implementa la interfaz por monitor serie. Permite enviar instrucciones desde la computadora para probar el robot, iniciar misiones y consultar información.

Funciones importantes:

* `iniciarComunicacion()`: inicia el puerto serie.
* `mostrarAyuda()`: imprime la lista de comandos.
* `leerComandos()`: lee e interpreta comandos del usuario.
* `reportar()`: imprime mensajes con formato común.

Comandos importantes:

* `G x y`: inicia una misión en celdas.
* `GC x y`: inicia una misión usando centímetros.
* `S`: detiene el robot.
* `D`: solicita descarga.
* `ESTADO`: muestra el estado actual.
* `MAPA`: imprime el mapa.
* `RUTA`: imprime la ruta.
* `E`, `IMU`, `U`, `SERVO`: muestran datos de diagnóstico.

Este módulo fue fundamental durante el desarrollo porque permitió probar cada subsistema por separado.

### `wifi_comandos`

El módulo `wifi_comandos` implementa una interfaz web para controlar el robot. El ESP32-S3 crea una red WiFi propia en modo Access Point y levanta un servidor web.

Funciones importantes:

* `iniciarWiFiComandos()`: crea el Access Point y configura el servidor.
* `actualizarWiFiComandos()`: atiende las solicitudes web.
* `ejecutarComandoWiFi()`: interpreta comandos recibidos desde la página.
* `generarEstadoRobot()`: genera un resumen del estado actual.
* `paginaHTML()`: construye la interfaz web.

La interfaz permite controlar el robot desde un celular o computadora, con botones de avance, retroceso, giro, STOP, misiones rápidas, sensores y diagnóstico.

Este módulo fue importante para la demostración, ya que permite controlar el robot sin depender exclusivamente del monitor serie.

Una limitación de esta implementación es que varias acciones de movimiento son bloqueantes. Mientras el robot ejecuta un avance o giro, el servidor puede tardar en responder nuevas solicitudes. Para una versión futura, se podría implementar una lógica no bloqueante o basada en estados para que la interfaz web permanezca más reactiva durante los movimientos.

### `main.ino`

El archivo `main.ino` es el punto de entrada del programa. Se mantiene intencionalmente corto para delegar la lógica en los módulos correspondientes.

En `setup()` se inicializa la comunicación, el robot y el sistema WiFi.

En `loop()` se ejecutan continuamente tres tareas principales:

* Leer comandos por serie.
* Atender comandos WiFi.
* Actualizar la lógica principal del robot.

Esto permite que el programa principal sea simple y que el comportamiento complejo quede organizado dentro de los módulos específicos.
