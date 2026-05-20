# Suposición de trabajo

Te propongo diseñarlo en dos capas:

una base robusta y simple, para que el robot ande bien y sea depurable,
pero con estructura compatible con un modo más competitivo después.

O sea: no haré un pseudocódigo “mínimo escolar”, sino uno modular y escalable.

*** Antes de escribir el pseudocódigo completo, te dejo unas decisiones de arquitectura que yo tomaría ***
## Sensores laterales IR

Haría la lectura así:

apagar LED IR
esperar muy poco
leer ADC = ambient
encender LED IR
esperar muy poco
leer ADC = active
señal útil = active - ambient

Eso te da una lectura bastante limpia.

## AS5600

Mantendría por cada rueda:

ángulo actual
ángulo previo
delta angular corregido por wraparound
suma total de ángulo
velocidad angular estimada

Porque el AS5600 mide de 0 a 360°, así que debes corregir salto de 359° a 0° y viceversa.

Estimación de movimiento

Usaría:

AS5600 como fuente principal de distancia lineal
MPU6050 giro Z como fuente principal de rotación instantánea
combinación de ambos para control y debugging

O sea:

para avanzar recto: prioridad a encoders + corrección con gyro
para girar: prioridad a gyro + validación con encoders
Máquina de estados

## Tu firmware debería quedar separado en:

init
calibración
espera
exploración
alineación
avance una celda
giro 90°
giro 180°
frenado / error

Eso hace muchísimo más fácil depurar.

# Decisiones de estructura

* La idea es separar claramente:

hardware y variables globales
inicialización
calibración
lectura de sensores
odometría
control de movimiento
primitivas de movimiento
(avanzar una celda, girar 90°, girar 180°, alinear)
detección de paredes
exploración del laberinto
modo speed-run
debug y seguridad

## 1. Arquitectura general

Tu robot debería trabajar con varios lazos:

lazo rápido: 1 kHz aprox
Lee encoders, IMU, estima velocidad, actualiza control motores
lazo medio: 100–200 Hz
Lee sensores IR y ToF, filtra, detecta paredes
lazo lento: 20–50 Hz
Máquina de estados, navegación, decisiones

## 2. Suposiciones de diseño

Se asumen estas decisiones:

tracción diferencial
dos ruedas motrices
apoyo con teflón
avance centrado por odometría y paredes
giros controlados principalmente por gyro
distancia lineal controlada principalmente por AS5600
una celda estándar de micromouse con:
``` psc
CELL_SIZE_MM = 180
esto puedes cambiarlo luego
diámetro de rueda:
WHEEL_DIAMETER_MM = 28
radio:
WHEEL_RADIUS_MM = 14
perímetro:
WHEEL_CIRCUMFERENCE_MM = PI * 28
```
## 3. Filosofía de control

Yo te recomiendo esta lógica:

Para avanzar recto

Usar:

distancia por encoders AS5600
corrección de rumbo por gyro
refinamiento lateral por sensores IR si hay pared lateral confiable
Para girar

Usar:

gyro como referencia principal del ángulo
encoders como chequeo secundario
Para centrarte en pasillos

Usar:

error lateral entre izquierda y derecha
solo si ambos sensores laterales están viendo pared de forma confiable
Para aproximación frontal

Usar:

ToF frontal
combinado con reducción progresiva de velocidad

# Pseudocodigo