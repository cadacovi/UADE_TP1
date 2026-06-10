## Módulo `config.h`

### Descripción general

El archivo `config.h` concentra las constantes globales del proyecto. Su función principal es centralizar en un único lugar la configuración de pines, dimensiones físicas, parámetros de navegación, umbrales de sensores, constantes de control, configuración del servo, comunicación WiFi y opciones de depuración.

Este módulo no implementa lógica de funcionamiento, sino que define los valores que luego son utilizados por el resto del programa. De esta forma, si se necesita modificar un pin, ajustar una velocidad, cambiar el tamaño de celda o recalibrar un parámetro de movimiento, puede hacerse desde este archivo sin tener que buscar el valor en distintos módulos del código.

### Constantes principales

#### Comunicación serie

* `SERIAL_BAUDRATE`: define la velocidad de comunicación serie utilizada para depuración y envío de comandos por monitor serie.

#### Pinout del ESP32-S3

En este archivo se definen los pines asociados a cada periférico del robot:

* `PIN_SDA` y `PIN_SCL`: pines usados para comunicación I2C con el MPU6050.
* `TRIG_FRONT`, `ECHO_FRONT`, `TRIG_LEFT`, `ECHO_LEFT`, `TRIG_RIGHT`, `ECHO_RIGHT`: pines de disparo y lectura de los tres sensores ultrasónicos.
* `ENCODER_LEFT` y `ENCODER_RIGHT`: entradas digitales asociadas a los encoders ópticos.
* `PIN_NSLEEP`, `AIN1`, `AIN2`, `BIN1`, `BIN2`: pines de control del driver DRV8833.
* `SERVO_PIN`: pin de señal PWM utilizado para controlar el servo MG90S.

#### Mapa local y sistema de coordenadas

* `TAM_CELDA_MM`: define el tamaño de cada celda del mapa local. En la versión actual, cada celda representa 250 mm, es decir, 25 cm.
* `MAPA_FILAS` y `MAPA_COLUMNAS`: definen las dimensiones de la matriz usada como mapa local temporal.
* `MAPA_ORIGEN_X` y `MAPA_ORIGEN_Y`: indican la posición del origen lógico dentro de la matriz.

El sistema de coordenadas local toma como referencia la posición inicial del robot. Al comenzar una misión, el robot se considera ubicado en la posición lógica `(0, 0)` y orientado hacia el `NORTE`. El eje `y` positivo representa el avance respecto al inicio de la misión, mientras que el eje `x` positivo representa desplazamiento hacia la derecha.

#### Umbrales de obstáculos

* `DIST_OBSTACULO_FRENTE_MM`: distancia mínima frontal para considerar que existe un obstáculo delante del robot.
* `DIST_OBSTACULO_LATERAL_MM`: distancia mínima lateral para considerar obstáculos a izquierda o derecha.
* `DIST_ULTRASONICO_MAX_MM`: distancia máxima considerada válida para las lecturas de los HC-SR04.
* `ULTRASONICO_MUESTRAS`: cantidad de muestras utilizadas para filtrar las mediciones ultrasónicas.

Estos valores permiten decidir cuándo una celda debe marcarse como ocupada o cuándo el robot debe detenerse y replanificar.

#### Constantes físicas y calibración

* `DIAMETRO_RUEDA_MM`: diámetro estimado de las ruedas.
* `PULSOS_POR_VUELTA`: cantidad teórica de pulsos por vuelta del encoder.
* `DISTANCIA_ENTRE_RUEDAS_MM`: separación aproximada entre las ruedas izquierda y derecha.
* `PULSOS_POR_MM`: valor calibrado empíricamente para convertir pulsos de encoder en distancia recorrida.
* `PULSOS_POR_CELDA`: cantidad de pulsos necesarios para avanzar una celda completa.

El parámetro más importante de esta sección es `PULSOS_POR_MM`, ya que fue ajustado experimentalmente para que la distancia estimada por los encoders coincida mejor con el desplazamiento real del robot.

#### Perfil de batería y PWM

El archivo incluye un perfil de PWM adaptado a una condición de batería baja, activado mediante:

* `PERFIL_BATERIA_BAJA`

Según este perfil se definen valores como:

* `PWM_MIN`
* `PWM_MAX`
* `PWM_BASE_AVANCE`
* `PWM_BASE_GIRO`
* `PWM_FINAL_AVANCE`
* `PWM_FINAL_GIRO`
* `PWM_AVANCE_MIN`
* `PWM_AVANCE_MAX`
* `PWM_GIRO_MIN`
* `PWM_GIRO_MAX`

Estos parámetros determinan las velocidades de avance y giro del robot. Al tenerlos centralizados, se facilita la calibración del movimiento según el estado de las baterías, el peso del robot y la fricción del suelo.

#### Control de avance

* `TOLERANCIA_DISTANCIA_MM`: margen aceptable de error al avanzar una distancia.
* `DISTANCIA_FRENADO_MM`: distancia restante a partir de la cual se reduce el PWM para frenar de forma más controlada.
* `KP_ENCODER_AVANCE`: ganancia proporcional para corregir diferencias entre las ruedas usando encoders.
* `KP_YAW_AVANCE`: ganancia proporcional para corregir desviaciones de orientación usando la IMU.
* `TIMEOUT_AVANCE_CELDA_MS`: tiempo máximo permitido para avanzar una celda.
* `MAX_CELDAS_TRAMO_RECTO`: cantidad máxima de celdas que el robot puede avanzar de forma continua en un mismo tramo recto.

Estas constantes son utilizadas por el módulo de control de movimiento para lograr avances más rectos, estables y repetibles.

#### Control de giro

* `TOLERANCIA_GIRO_GRADOS`: error angular aceptable al realizar un giro.
* `TOLERANCIA_GIRO_FINO_GRADOS`: tolerancia usada para correcciones finas posteriores al giro.
* `KP_GIRO`: ganancia proporcional del control de giro.
* `TIMEOUT_GIRO_MS`: tiempo máximo permitido para completar un giro.
* `TOLERANCIA_CORRECCION_YAW` y `MAX_CORRECCION_YAW`: parámetros usados para limitar la corrección angular durante el avance.

Estos valores permiten controlar los giros mediante el yaw relativo entregado por la IMU.

#### IMU MPU6050

* `MPU6050_ADDR`: dirección I2C del sensor.
* `TIEMPO_CALIBRACION_IMU_MS`: tiempo de calibración inicial del giroscopio.
* `MUESTRAS_CALIBRACION_IMU`: cantidad de muestras usadas para estimar el offset del giroscopio.

Estos parámetros son importantes para reducir el error inicial del yaw relativo.

#### Servo de descarga

* `SERVO_ANGULO_CERRADO`: posición del servo con el mecanismo cerrado.
* `SERVO_ANGULO_ABIERTO`: posición del servo con el mecanismo abierto.
* `TIEMPO_APERTURA_SERVO_MS`: tiempo durante el cual el servo permanece abierto para realizar la descarga.
* `SERVO_MIN_US`, `SERVO_MAX_US` y `SERVO_FRECUENCIA_HZ`: parámetros de señal PWM usados por la librería del servo.

#### WiFi y control manual

* `WIFI_AP_SSID`: nombre de la red WiFi creada por el ESP32-S3.
* `WIFI_AP_PASSWORD`: contraseña de la red.
* `WIFI_AP_CANAL`, `WIFI_AP_OCULTO` y `WIFI_AP_MAX_CLIENTES`: parámetros de configuración del Access Point.
* `WIFI_MANUAL_AVANCE_MM`: distancia de avance usada por los comandos manuales desde la interfaz web.
* `WIFI_MANUAL_GIRO_GRADOS`, `WIFI_MANUAL_GIRO_MEDIO_GRADOS` y `WIFI_MANUAL_GIRO_FINO_GRADOS`: ángulos usados para giros manuales desde la interfaz web.

#### Depuración

El archivo también define banderas de depuración:

* `DEBUG_GENERAL`
* `DEBUG_COMANDOS`
* `DEBUG_ESTADOS`
* `DEBUG_SENSORES`
* `DEBUG_CONTROL`
* `DEBUG_MAPA`

Estas banderas permiten activar o desactivar mensajes de diagnóstico según la parte del sistema que se esté probando.

### Importancia dentro del proyecto

`config.h` es uno de los archivos más importantes del proyecto porque permite adaptar el comportamiento del robot sin modificar la lógica interna de cada módulo. Esto fue especialmente útil durante la etapa de calibración, ya que parámetros como el tamaño de celda, los PWM, las tolerancias, los umbrales de obstáculos y la relación `PULSOS_POR_MM` debieron ajustarse mediante pruebas reales.
