## Módulo `motores`

### Descripción general

El módulo `motores` se encarga del control directo de los motores DC del robot mediante el driver DRV8833. Su función principal es abstraer el manejo de los pines del driver, permitiendo controlar cada motor a partir de un valor PWM con signo.

De esta manera, el resto del programa no necesita escribir directamente sobre los pines `AIN1`, `AIN2`, `BIN1` y `BIN2`, sino que puede llamar a funciones de mayor nivel como `setMotores(...)`, `detenerMotores()` o `frenarSuave()`.

Este módulo representa una capa de bajo nivel dentro del software, ya que se comunica directamente con el hardware de potencia encargado del movimiento.

### Funciones principales

#### `inicializarMotores()`

Configura como salidas los pines asociados al driver DRV8833:

* `PIN_NSLEEP`
* `AIN1`
* `AIN2`
* `BIN1`
* `BIN2`

Luego activa el driver colocando `PIN_NSLEEP` en estado alto, detiene ambos motores y envía un mensaje por el monitor serie indicando que el módulo fue inicializado.

Esta función se ejecuta durante la inicialización general del robot.

#### `setMotorIzquierdo(int pwm)`

Controla individualmente el motor izquierdo.

El parámetro `pwm` puede ser positivo, negativo o cero:

* Si `pwm > 0`, el motor gira en sentido de avance.
* Si `pwm < 0`, el motor gira en sentido inverso.
* Si `pwm == 0`, el motor se detiene.

Internamente, la función aplica la señal PWM sobre los pines `AIN1` y `AIN2` del DRV8833.

#### `setMotorDerecho(int pwm)`

Controla individualmente el motor derecho con la misma lógica usada para el motor izquierdo.

El valor de `pwm` determina sentido y velocidad:

* PWM positivo: avance.
* PWM negativo: retroceso.
* PWM cero: motor detenido.

En este caso se utilizan los pines `BIN1` y `BIN2` del DRV8833.

#### `setMotores(int pwmIzquierdo, int pwmDerecho)`

Permite controlar ambos motores al mismo tiempo. Recibe un PWM para el motor izquierdo y otro para el motor derecho.

Esta función es utilizada por módulos de mayor nivel, como `control_movimiento`, para ejecutar avances, retrocesos y giros diferenciales.

Por ejemplo:

* Ambos PWM positivos: avance.
* Ambos PWM negativos: retroceso.
* Motor izquierdo negativo y derecho positivo: giro hacia la izquierda.
* Motor izquierdo positivo y derecho negativo: giro hacia la derecha.

#### `detenerMotores()`

Detiene ambos motores colocando el PWM de ambos lados en cero.

Es una función importante de seguridad, ya que permite detener el robot ante finalización de movimiento, detección de obstáculo, timeout o comando de parada.

#### `frenarSuave()`

Reduce progresivamente la velocidad de ambos motores antes de detenerlos completamente.

En lugar de pasar directamente de un PWM alto a cero, aplica una secuencia decreciente de valores PWM. Esto ayuda a disminuir frenadas bruscas, reduciendo deslizamientos, errores de orientación y movimientos repentinos.

#### `frenarSuaveDesde(int pwmInicial)`

Realiza un frenado progresivo partiendo desde un valor PWM inicial recibido como parámetro.

Esta función es útil cuando el módulo de control de movimiento conoce la velocidad actual y desea detener el robot de forma gradual desde ese valor.

### Funciones de prueba

El módulo incluye funciones simples para verificar el funcionamiento de motores y cableado:

* `pruebaAvanzar()`: mueve ambos motores hacia adelante.
* `pruebaRetroceder()`: mueve ambos motores hacia atrás.
* `pruebaGirarIzquierda()`: hace girar el robot hacia la izquierda mediante movimiento diferencial.
* `pruebaGirarDerecha()`: hace girar el robot hacia la derecha mediante movimiento diferencial.

Estas funciones fueron útiles durante la etapa de pruebas para comprobar el sentido de giro de los motores, la conexión al DRV8833 y la respuesta básica del sistema de tracción.

### Constantes relacionadas

Este módulo utiliza varias constantes definidas en `config.h`:

* `PIN_NSLEEP`: pin que habilita el driver DRV8833.
* `AIN1` y `AIN2`: pines de control del motor izquierdo.
* `BIN1` y `BIN2`: pines de control del motor derecho.
* `PWM_MAX`: valor máximo permitido para la señal PWM.
* `PWM_BASE_AVANCE`: velocidad base usada en pruebas de avance y retroceso.
* `PWM_BASE_GIRO`: velocidad base usada en pruebas de giro.

También se utiliza una función interna llamada `limitarPWM(int pwm)`, que limita el valor de PWM para que no supere el rango permitido. Esto evita enviar valores fuera del rango esperado al driver.

### Relación con otros módulos

El módulo `motores` es utilizado principalmente por `control_movimiento`, que implementa movimientos más complejos como avanzar una distancia determinada, girar cierto ángulo, corregir trayectoria y detenerse ante obstáculos.

También puede ser usado por `comandos` o `wifi_comandos` para ejecutar pruebas o movimientos manuales solicitados desde el monitor serie o desde la interfaz web.

### Comentarios de diseño

La decisión de representar el PWM con signo simplifica mucho el control de los motores. Un único valor entero permite indicar tanto la velocidad como el sentido de giro. Esto hace que las funciones de movimiento de mayor nivel sean más claras y evita repetir lógica de control del DRV8833 en distintas partes del programa.

Además, el módulo separa el control básico de motores del control de movimiento. Es decir, `motores` no decide cuánto debe avanzar el robot ni cómo corregir la trayectoria; solamente ejecuta las órdenes eléctricas sobre el driver. Esa separación mejora la organización del código y permite modificar el control de bajo nivel sin afectar directamente la lógica de navegación.
