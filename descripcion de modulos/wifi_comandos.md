## Módulo `wifi_comandos`

### Descripción general

El módulo `wifi_comandos` se encarga de implementar la comunicación inalámbrica del robot mediante WiFi. El ESP32-S3 crea una red propia en modo Access Point y ejecuta un servidor web al que el usuario puede acceder desde un navegador.

A través de esta interfaz web se pueden enviar comandos al robot, ejecutar movimientos manuales, iniciar misiones, consultar estado, leer sensores y detener el sistema mediante un botón de STOP.

Este módulo cumple una función similar al módulo `comandos`, pero en lugar de usar el monitor serie, utiliza una página web servida directamente por el ESP32-S3.

### Funcionamiento general

El módulo crea un servidor web en el puerto 80:

```cpp
static WebServer servidor(80);
```

También mantiene un puntero al objeto principal del robot:

```cpp
static Robot* robotWiFi = nullptr;
```

Ese puntero permite que las funciones internas del servidor puedan llamar métodos de la clase `Robot`, como crear una misión, detener el robot, solicitar descarga o consultar su estado.

El flujo general es:

1. El ESP32-S3 crea una red WiFi propia.
2. El usuario se conecta a esa red.
3. El usuario abre la dirección IP del ESP32-S3 en el navegador.
4. El servidor entrega una página HTML de control.
5. Los botones de la página envían comandos HTTP a la ruta `/cmd`.
6. El ESP32-S3 interpreta esos comandos y ejecuta las acciones correspondientes.

### Variables principales

* `servidor`: objeto `WebServer` que escucha peticiones HTTP en el puerto 80.
* `robotWiFi`: puntero al objeto `Robot` que será controlado desde la interfaz web.
* `ultimaRespuesta`: texto de la última respuesta generada por el sistema, mostrado en la página web.

Estas variables se declaran como `static`, por lo que quedan limitadas al archivo `wifi_comandos.cpp`.

### Funciones principales

#### `iniciarWiFiComandos(Robot &robot)`

Inicializa el sistema WiFi y el servidor web.

Primero guarda la referencia al robot:

```cpp
robotWiFi = &robot;
```

Luego configura el ESP32-S3 en modo Access Point:

```cpp
WiFi.mode(WIFI_AP);
```

Después crea la red WiFi usando las constantes definidas en `config.h`:

* `WIFI_AP_SSID`
* `WIFI_AP_PASSWORD`
* `WIFI_AP_CANAL`
* `WIFI_AP_OCULTO`
* `WIFI_AP_MAX_CLIENTES`

Si el Access Point se crea correctamente, imprime por monitor serie el SSID, contraseña e IP del servidor.

Finalmente, registra las rutas del servidor:

* `/`: muestra la página web principal.
* `/cmd`: recibe comandos enviados desde la interfaz.

Luego inicia el servidor mediante `servidor.begin()`.

#### `actualizarWiFiComandos()`

Debe llamarse periódicamente desde el `loop` principal.

Ejecuta:

```cpp
servidor.handleClient();
```

Esta función permite que el servidor atienda solicitudes entrantes del navegador. Sin esta llamada periódica, la interfaz web no respondería correctamente a los comandos del usuario.

#### `paginaHTML()`

Genera el contenido HTML de la interfaz web.

La página incluye:

* Título del proyecto.
* Botón grande de STOP.
* Panel de control manual.
* Botones de avance, retroceso y giros.
* Correcciones de giro de 45° y 10°.
* Misiones rápidas.
* Entrada manual de comandos.
* Botones de acciones como estado, sensores, descarga y ayuda.
* Botones de depuración, como reset de yaw, reset de encoders, avance de celda y giro de 180°.
* Área de respuesta donde se muestra el resultado del último comando.

La página también incluye código JavaScript que envía comandos al endpoint `/cmd` mediante `fetch()`.

#### `manejarRaiz()`

Manejador de la ruta principal `/`.

Cuando el usuario entra a la IP del robot desde el navegador, esta función responde con la página HTML generada por `paginaHTML()`.

#### `manejarComando()`

Manejador de la ruta `/cmd`.

Esta función espera recibir un parámetro llamado `c`, que contiene el comando a ejecutar. Por ejemplo:

```text
/cmd?c=ESTADO
```

Si el parámetro no existe, responde con error. Si existe, llama a:

```cpp
ultimaRespuesta = ejecutarComandoWiFi(comando);
```

Luego envía la respuesta como texto plano al navegador.

### Interpretación de comandos WiFi

#### `ejecutarComandoWiFi(String comando)`

Es la función que interpreta los comandos recibidos desde la interfaz web.

Primero verifica que el puntero `robotWiFi` sea válido. Luego limpia el texto recibido, lo convierte a mayúsculas y compara el comando con las opciones disponibles.

Entre los comandos principales se encuentran:

* `AYUDA`: devuelve la lista de comandos disponibles.
* `ESTADO`: genera un resumen del estado actual del robot.
* `G x y`: crea una nueva misión en coordenadas de celdas.
* `GC x_cm y_cm`: crea una nueva misión usando coordenadas en centímetros.
* `S`: detiene el robot.
* `D`: solicita descarga manual.
* `AV100`: avanza 100 mm.
* `AV200`: avanza 200 mm.
* `AVCELDA`: avanza una celda.
* `GDER`: gira 90° a la derecha.
* `GIZQ`: gira 90° a la izquierda.
* `G180`: gira 180°.
* `ER`: reinicia encoders.
* `YAW0`: reinicia yaw relativo.
* `U`: devuelve lecturas de sensores ultrasónicos.

Además, incluye comandos específicos para control manual desde la interfaz web:

* `MAN_AV`: avance manual.
* `MAN_RET`: retroceso manual.
* `MAN_DER`: giro manual a la derecha de 90°.
* `MAN_IZQ`: giro manual a la izquierda de 90°.
* `MAN_DER45`: giro manual a la derecha de 45°.
* `MAN_IZQ45`: giro manual a la izquierda de 45°.
* `MAN_DER10`: corrección fina a la derecha de 10°.
* `MAN_IZQ10`: corrección fina a la izquierda de 10°.

Estos comandos llaman a funciones del módulo `control_movimiento`, como `avanzarDistanciaResultado()` y `girarAnguloResultado()`.

### Generación de estado

#### `generarEstadoRobot()`

Devuelve un texto con información actual del robot, incluyendo:

* Estado actual.
* Posición lógica.
* Destino.
* Dirección.
* Yaw relativo.
* Pulsos de encoders izquierdo y derecho.
* Distancia estimada de cada rueda.

Esta función permite mostrar información de diagnóstico desde la interfaz web sin depender del monitor serie.

### Funciones auxiliares

#### `estadoComoTextoWiFi(EstadoRobot estado)`

Convierte un estado del robot en texto legible para mostrarlo desde la interfaz web.

#### `direccionComoTextoWiFi(Direccion direccion)`

Convierte una dirección lógica en texto legible.

#### `extraerDosEnteros(String comando, int &a, int &b)`

Extrae dos enteros desde comandos como `G x y` o `GC x y`.

Cumple la misma función que la versión usada en el módulo de comandos serie, pero definida localmente para este módulo.

### Interfaz web

La interfaz web se construye directamente como una cadena HTML dentro del firmware. Incluye estilos CSS y funciones JavaScript simples.

El usuario puede interactuar con el robot mediante botones o escribiendo comandos manualmente.

Los botones llaman a una función JavaScript:

```javascript
cmd('COMANDO')
```

Esa función envía una petición HTTP al servidor:

```javascript
fetch('/cmd?c=' + encodeURIComponent(c))
```

Luego la respuesta del ESP32-S3 se muestra en el bloque de respuesta de la página.

Esta implementación permite controlar el robot desde un celular o computadora sin instalar una aplicación externa.

### Constantes relacionadas

Este módulo utiliza constantes definidas en `config.h`:

* `WIFI_AP_SSID`: nombre de la red creada por el robot.
* `WIFI_AP_PASSWORD`: contraseña del Access Point.
* `WIFI_AP_CANAL`: canal WiFi utilizado.
* `WIFI_AP_OCULTO`: define si la red se muestra u oculta.
* `WIFI_AP_MAX_CLIENTES`: cantidad máxima de clientes conectados.
* `WIFI_MANUAL_AVANCE_MM`: distancia usada para avance y retroceso manual.
* `WIFI_MANUAL_GIRO_GRADOS`: ángulo de giro manual principal.
* `WIFI_MANUAL_GIRO_MEDIO_GRADOS`: ángulo usado para correcciones de 45°.
* `WIFI_MANUAL_GIRO_FINO_GRADOS`: ángulo usado para correcciones finas de 10°.

### Relación con otros módulos

El módulo `wifi_comandos` se relaciona principalmente con:

* `Robot`: para crear misiones, detener el robot, consultar estado y solicitar descarga.
* `control_movimiento`: para ejecutar movimientos manuales y giros.
* `encoders`: para consultar o reiniciar pulsos.
* `imu`: para consultar yaw o reiniciarlo.
* `ultrasonidos`: para mostrar lecturas de distancia.
* `motores`: para detener el robot.
* `servo_carga`: para acciones relacionadas con la descarga.

### Comentarios de diseño

La decisión de implementar WiFi en modo Access Point permite que el robot funcione sin depender de una red externa. El ESP32-S3 crea su propia red, por lo que el usuario puede conectarse directamente desde un celular o computadora durante la demostración.

La interfaz web facilita la presentación del proyecto, ya que permite controlar el robot mediante botones claros en lugar de depender únicamente del monitor serie. También mejora la seguridad operativa al incluir un botón STOP visible.

El uso de comandos de texto detrás de los botones mantiene compatibilidad conceptual con el módulo `comandos`, ya que muchas acciones son equivalentes a las disponibles por puerto serie.

Una limitación de esta implementación es que varias acciones de movimiento son bloqueantes. Mientras el robot ejecuta un avance o giro, el servidor puede tardar en responder nuevas solicitudes. Para una versión futura, se podría implementar una lógica no bloqueante o basada en estados para que la interfaz web permanezca más reactiva durante los movimientos.
