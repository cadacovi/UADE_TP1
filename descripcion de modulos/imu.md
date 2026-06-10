## Módulo `imu`

### Descripción general

El módulo `imu` se encarga de la comunicación con la IMU MPU6050 mediante el bus I2C. En este proyecto se utiliza principalmente el giroscopio en el eje Z para estimar el cambio de orientación del robot, es decir, su yaw relativo.

El yaw relativo permite saber cuánto giró el robot respecto a una orientación de referencia. Esta información es utilizada por el sistema de control de movimiento para realizar giros de ángulo determinado y para corregir desviaciones durante el avance recto.

A diferencia de una brújula, el MPU6050 no entrega una orientación absoluta respecto al norte magnético. En este proyecto se trabaja con orientación relativa: al iniciar una misión o antes de un giro, el yaw puede reiniciarse a cero, y a partir de ahí se mide cuánto giró el robot.

### Funcionamiento general

El MPU6050 entrega una velocidad angular en el eje Z. Esta velocidad angular se expresa en grados por segundo. Para obtener el ángulo girado, el programa integra esa velocidad angular en el tiempo.

La idea básica es:

```cpp
yawRelativo += gyroZ * dt;
```

Donde:

* `gyroZ` es la velocidad angular corregida en grados por segundo.
* `dt` es el tiempo transcurrido desde la última actualización, expresado en segundos.
* `yawRelativo` es el ángulo acumulado desde el último reinicio del yaw.

### Variables principales

* `offsetGyroZ`: valor de corrección del giroscopio en el eje Z. Se obtiene durante la calibración inicial.
* `gyroZ`: velocidad angular actual en el eje Z, expresada en grados por segundo.
* `yawRelativo`: ángulo relativo acumulado, expresado en grados.
* `tiempoAnteriorIMU`: instante de la última actualización de la IMU, usado para calcular `dt`.
* `imuInicializada`: indica si la IMU fue detectada e inicializada correctamente.

Estas variables se declaran como `static`, por lo que quedan limitadas al archivo `imu.cpp` y no pueden ser modificadas directamente desde otros módulos. El acceso externo se realiza mediante funciones como `obtenerYawRelativo()` y `obtenerGyroZ()`.

### Funciones principales

#### `inicializarIMU()`

Inicializa la comunicación I2C usando los pines definidos en `config.h`. Luego verifica si el MPU6050 responde en la dirección configurada.

Si el sensor no responde, se imprime un mensaje de error, se marca la IMU como no inicializada y la función devuelve `false`.

Si el sensor responde correctamente, se realizan las siguientes acciones:

* Se despierta el MPU6050 escribiendo en el registro `PWR_MGMT_1`.
* Se configura el giroscopio en el rango de ±250 grados por segundo.
* Se marca la IMU como inicializada.
* Se guarda el tiempo inicial con `micros()`.
* Se ejecuta la calibración del giroscopio.
* Se reinicia el yaw relativo.

La función devuelve `true` si la inicialización fue exitosa.

#### `calibrarIMU()`

Calcula el offset del giroscopio en el eje Z.

Durante esta calibración, el robot debe permanecer quieto. El programa toma varias muestras del registro `GYRO_ZOUT_H`, las promedia y convierte el valor obtenido a grados por segundo usando la sensibilidad correspondiente al rango ±250 °/s.

La constante utilizada para esa conversión es:

```cpp
131.0
```

Esto se debe a que, para el rango de ±250 °/s, el MPU6050 entrega aproximadamente 131 LSB por cada grado por segundo.

El valor resultante se guarda en `offsetGyroZ` y luego se resta a las futuras mediciones del giroscopio. Esto reduce el error causado por pequeñas lecturas no nulas cuando el robot está quieto.

#### `actualizarIMU()`

Actualiza las variables internas de la IMU.

Primero calcula el tiempo transcurrido desde la última actualización:

```cpp
dt = (tiempoActual - tiempoAnteriorIMU) / 1000000.0;
```

Luego lee el valor crudo del giroscopio en Z, lo convierte a grados por segundo y le resta el offset calibrado:

```cpp
gyroZ = (rawGyroZ / 131.0) - offsetGyroZ;
```

Finalmente integra la velocidad angular para actualizar el yaw relativo:

```cpp
yawRelativo += gyroZ * dt;
```

Esta función debe llamarse periódicamente mientras se necesite mantener actualizada la orientación del robot.

#### `resetYaw()`

Reinicia el yaw relativo a cero y actualiza el tiempo de referencia.

Esta función es útil antes de iniciar un giro o al comenzar una nueva misión. De esta manera, el robot puede medir el cambio angular desde una referencia conocida.

#### `obtenerYawRelativo()`

Devuelve el valor actual de `yawRelativo`, expresado en grados.

Este valor es utilizado por el control de movimiento para saber cuánto giró el robot respecto a la última referencia.

#### `obtenerGyroZ()`

Devuelve la velocidad angular actual en el eje Z, expresada en grados por segundo.

Puede utilizarse para diagnóstico o para observar si el sensor detecta correctamente los giros del robot.

#### `imprimirIMU()`

Actualiza la lectura de la IMU y luego imprime por monitor serie:

* Velocidad angular en Z.
* Yaw relativo.
* Offset calibrado del giroscopio.

Esta función resulta útil para pruebas, depuración y verificación del funcionamiento del sensor.

### Función auxiliar interna

#### `leerRegistro16(uint8_t registroAlto)`

Lee dos bytes consecutivos del MPU6050 mediante I2C y los combina en un valor entero de 16 bits con signo. El MPU6050 no te entrega el valor del giroscopio Z como “un número completo” de una vez. Lo guarda en dos pedacitos: GYRO_ZOUT_H "byte alto" y GYRO_ZOUT_L "byte bajo"

Se utiliza para leer registros como `GYRO_ZOUT_H`, que almacenan mediciones de 16 bits distribuidas en dos registros: byte alto y byte bajo.

Esta función está declarada como `static`, por lo que solo puede usarse dentro de `imu.cpp`.

### Constantes relacionadas

Este módulo utiliza varias constantes definidas en `config.h`:

* `PIN_SDA`: pin SDA del bus I2C.
* `PIN_SCL`: pin SCL del bus I2C.
* `MPU6050_ADDR`: dirección I2C del MPU6050.
* `MUESTRAS_CALIBRACION_IMU`: cantidad de muestras utilizadas para calcular el offset del giroscopio.

Además, dentro del código se utiliza el valor `131.0`, correspondiente a la sensibilidad del giroscopio en el rango ±250 °/s.

### Relación con otros módulos

El módulo `imu` es utilizado principalmente por `control_movimiento`.

Durante los giros, el control de movimiento consulta el yaw relativo para determinar cuándo el robot alcanzó el ángulo objetivo. Durante el avance recto, la IMU también permite corregir desviaciones de orientación, complementando la corrección basada en encoders.

También puede ser utilizado por `comandos` o `wifi_comandos` para mostrar datos de diagnóstico al usuario.

### Comentarios de diseño

Una decisión importante fue utilizar el yaw relativo obtenido por integración del giroscopio, en lugar de intentar calcular una orientación absoluta. Esta estrategia es adecuada para trayectos cortos y giros controlados, ya que permite medir cambios de orientación con suficiente precisión para la navegación por celdas.

Sin embargo, al tratarse de una integración de velocidad angular, el yaw puede acumular error con el tiempo debido a deriva del sensor, ruido o calibración imperfecta. Por eso se utiliza principalmente en intervalos cortos, como giros de 90° o correcciones durante el avance.

También se incluye una calibración inicial del giroscopio, en la cual el robot debe permanecer quieto. Esta calibración permite estimar el offset del eje Z y reducir el error acumulado durante el uso.
