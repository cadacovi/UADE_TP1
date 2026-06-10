## Archivo principal `main.ino`

### Descripción general

El archivo `main.ino` es el programa principal que ejecuta el ESP32-S3. Su función es inicializar el sistema y mantener el ciclo principal de ejecución del robot.

A diferencia de otros archivos, `main.ino` no contiene la lógica detallada de sensores, motores, navegación o comunicación. En cambio, delega esas tareas a los módulos correspondientes. Esto permite que el programa principal sea simple, ordenado y fácil de entender.

### Inclusión de módulos

Al inicio del archivo se incluyen los módulos principales del proyecto:

```cpp
#include "config.h"
#include "tipos.h"
#include "robot.h"
#include "comandos.h"
#include "wifi_comandos.h"
```

Estos archivos permiten acceder a la configuración general, tipos compartidos, clase `Robot`, comandos por puerto serie y comandos por WiFi.

### Objeto principal del robot

Se declara un objeto global:

```cpp
Robot robot;
```

Este objeto representa el robot completo y concentra el control de misión, estado, navegación, mapa, sensores y actuadores mediante los módulos internos.

### Función `setup()`

La función `setup()` se ejecuta una sola vez al encender o reiniciar el ESP32-S3.

En este proyecto realiza los siguientes pasos:

1. Inicia la comunicación serie mediante `iniciarComunicacion()`.
2. Reporta el inicio del robot.
3. Inicializa el objeto `robot` mediante `robot.inicializar()`.
4. Inicia el sistema WiFi y el servidor web mediante `iniciarWiFiComandos(robot)`.
5. Reporta que el robot está listo.

De esta forma, al finalizar `setup()`, el robot queda preparado para recibir comandos por monitor serie o desde la interfaz web.

### Función `loop()`

La función `loop()` se ejecuta continuamente mientras el ESP32-S3 está encendido.

En cada iteración realiza tres tareas principales:

```cpp
leerComandos(robot);        // Serial USB
actualizarWiFiComandos();   // WiFi
robot.actualizar();
```

Primero revisa si llegaron comandos por puerto serie. Luego atiende posibles solicitudes desde la interfaz WiFi. Finalmente actualiza la lógica principal del robot mediante la máquina de estados implementada en la clase `Robot`.

### Importancia dentro del proyecto

El archivo `main.ino` cumple el rol de coordinador principal del programa Arduino. Mantiene el ciclo de ejecución simple y delega la complejidad a los módulos especializados.

Esta estructura mejora la organización del código, ya que evita concentrar toda la lógica en el archivo principal. En lugar de eso, `main.ino` solamente inicializa el sistema y llama periódicamente a las funciones de actualización necesarias.
