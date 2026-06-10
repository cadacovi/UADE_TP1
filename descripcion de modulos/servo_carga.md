## Módulo `servo_carga`

### Descripción general

El módulo `servo_carga` se encarga de controlar el servo MG90S utilizado como mecanismo de descarga del robot. Su función principal es abstraer el manejo del servo, permitiendo abrir, cerrar o ejecutar una descarga completa mediante funciones simples.

Este módulo representa una capa de control de actuador. El resto del programa no necesita conocer los detalles de la señal PWM del servo, sino que puede llamar a funciones como `abrirCarga()`, `cerrarCarga()` o `descargarCarga()`.

### Funcionamiento general

El servo se controla mediante la librería `ESP32Servo`. Durante la inicialización se configura la frecuencia de trabajo del servo y se asocia el objeto `Servo` al pin definido en `config.h`.

El sistema trabaja con dos posiciones principales:

* Posición cerrada: el mecanismo mantiene la carga retenida.
* Posición abierta: el mecanismo libera la carga.

La función de descarga abre el servo durante un tiempo definido y luego vuelve a cerrarlo.

### Variables principales

* `servoCarga`: objeto de tipo `Servo` encargado de generar la señal PWM para el MG90S.
* `anguloActualServo`: variable que almacena el último ángulo enviado al servo.
* `servoInicializado`: bandera que indica si el servo fue inicializado correctamente.

Estas variables se declaran como `static`, por lo que solo pueden ser utilizadas dentro del archivo `servo_carga.cpp`. El resto del programa interactúa con el servo mediante las funciones públicas declaradas en `servo_carga.h`.

### Funciones principales

#### `inicializarServoCarga()`

Inicializa el servo de carga.

Primero configura la frecuencia de trabajo mediante:

```cpp
servoCarga.setPeriodHertz(SERVO_FRECUENCIA_HZ);
```

Luego asocia el servo al pin configurado, indicando también los pulsos mínimo y máximo permitidos:

```cpp
servoCarga.attach(SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);
```

Después llama a `cerrarCarga()` para dejar el mecanismo en su posición inicial cerrada, marca el servo como inicializado y muestra un mensaje por monitor serie.

Esta función se ejecuta durante la inicialización general del robot.

#### `abrirCarga()`

Mueve el servo hacia la posición abierta, definida por la constante `SERVO_ANGULO_ABIERTO`.

Antes de mover el servo, verifica si fue inicializado. Si el servo no está inicializado, imprime un mensaje de error y no ejecuta la apertura.

Al abrir la carga, actualiza la variable `anguloActualServo` e imprime un mensaje por monitor serie.

#### `cerrarCarga()`

Mueve el servo hacia la posición cerrada, definida por la constante `SERVO_ANGULO_CERRADO`.

También actualiza la variable `anguloActualServo` e imprime un mensaje por monitor serie.

Esta función se utiliza tanto durante la inicialización como después de completar una descarga.

#### `descargarCarga()`

Ejecuta la secuencia completa de descarga.

Primero verifica que el servo esté inicializado. Luego realiza los siguientes pasos:

1. Imprime un mensaje indicando que comienza la descarga.
2. Llama a `abrirCarga()`.
3. Espera el tiempo definido por `TIEMPO_APERTURA_SERVO_MS`.
4. Llama a `cerrarCarga()`.
5. Imprime un mensaje indicando que la descarga fue completada.

Esta función permite que el módulo principal del robot ejecute la acción de descarga sin preocuparse por los detalles temporales del servo.

#### `imprimirEstadoServo()`

Imprime por monitor serie el estado actual del servo:

* Ángulo actual.
* Si el servo está inicializado o no.

Es útil para pruebas, diagnóstico y verificación del mecanismo de descarga.

### Constantes relacionadas

Este módulo utiliza varias constantes definidas en `config.h`:

* `SERVO_PIN`: pin usado para la señal PWM del servo.
* `SERVO_ANGULO_CERRADO`: ángulo correspondiente a la posición cerrada.
* `SERVO_ANGULO_ABIERTO`: ángulo correspondiente a la posición abierta.
* `TIEMPO_APERTURA_SERVO_MS`: tiempo durante el cual la compuerta permanece abierta durante una descarga.
* `SERVO_MIN_US`: ancho de pulso mínimo usado por la librería.
* `SERVO_MAX_US`: ancho de pulso máximo usado por la librería.
* `SERVO_FRECUENCIA_HZ`: frecuencia PWM del servo, normalmente 50 Hz.

### Relación con otros módulos

El módulo `servo_carga` es utilizado principalmente por `robot`, que llama a `descargarCarga()` cuando el robot llega al destino de la misión.

También puede ser utilizado desde `comandos` o `wifi_comandos` para probar manualmente el mecanismo de apertura y cierre desde el monitor serie o desde la interfaz web.

### Comentarios de diseño

La decisión de encapsular el servo en un módulo propio permite separar la lógica de descarga de la lógica general de navegación. El módulo `robot` solo necesita indicar cuándo descargar, mientras que `servo_carga` se encarga de la posición, tiempos y control PWM del actuador.

El uso de constantes para los ángulos abierto y cerrado facilita la calibración mecánica del mecanismo. Si físicamente la compuerta queda desalineada, basta con ajustar los valores `SERVO_ANGULO_CERRADO` o `SERVO_ANGULO_ABIERTO` en `config.h`, sin modificar la lógica del programa.

La función `descargarCarga()` utiliza una espera bloqueante mediante `delay()`. En este proyecto es aceptable porque durante la descarga el robot no necesita desplazarse ni controlar trayectoria. Para una versión más avanzada, esta secuencia podría implementarse de manera no bloqueante mediante una máquina de estados.
