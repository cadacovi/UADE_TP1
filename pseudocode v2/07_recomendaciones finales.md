# 25. Orden recomendado de desarrollo

Yo no intentaría programar todo de golpe. Lo haría así:

Etapa 1
Mover motores con PWM y dirección.

Etapa 2
Leer ambos AS5600 de forma estable y comprobar:
dirección correcta
delta correcto
distancia recorrida correcta

Etapa 3
Leer MPU6050 y estimar yaw por integración de gyro.

Etapa 4
Implementar control de giro 90°.

Etapa 5
Implementar avance recto por distancia y corrección por gyro.

Etapa 6
Leer IR laterales con sustracción de ambiente.

Etapa 7
Leer ToF frontal y usarlo para alineación/frenado.

Etapa 8
Integrar detección de paredes y lógica de celdas.

Etapa 9
Implementar flood fill.

Etapa 10
Speed-run.

# 26. Observaciones técnicas sobre tu hardware

Ahora te doy opinión directa para que este pseudocódigo no se quede “bonito pero poco práctico”.

* AS5600 por I2C

Buena idea usar un bus por sensor. En software:

no leas ambos en la misma función bloqueante muy lenta
usa clock alto
mide el tiempo real de lectura
valida si a 1 kHz realmente llegas cómodo

Si no llegas, baja encoders a 500 Hz y deja el control interno interpolado.

* MPU6050

Te servirá para yaw, pero deriva con el tiempo.
En micromouse no pasa nada grave si:

recalibras antes de salir
no haces recorridos larguísimos sin corrección
usas encoders y paredes para corregir indirectamente

* IR laterales

Me gusta mucho que enciendas y apagues el LED.
Eso sí:

usa resistencias bien elegidas
coloca geometría repetible
intenta que el campo de visión lateral sea angosto
filtra bastante

* ToF frontal

Muy útil para:

confirmar pared frontal
parar a distancia consistente
alinear frente

No confiaría en él como única base de navegación longitudinal.

* Sin rueda loca

Puede salir muy bien. Pero debes probar:

rozamiento estático al arrancar
si “raspa” de más en giros
si el teflón delantero y trasero tienen la misma altura real
si el robot oscila al acelerar

# 27. Qué partes ajustaría después de las primeras pruebas

Seguro vas a terminar ajustando estas variables:

WHEEL_BASE_MM
LEFT_REFERENCE_VALUE
RIGHT_REFERENCE_VALUE
umbrales de pared lateral
bias del gyro
PID de heading
PID de giro
PID de velocidad de rueda
velocidad máxima útil real
aceleración máxima útil real

# 28. Versión resumida del flujo completo

El comportamiento global sería:

BOOT
-> inicializar hardware
-> calibrar gyro, IR, encoders
-> esperar botón

al iniciar:
-> explorar celda por celda
-> en cada celda:
   - sensar paredes
   - actualizar mapa
   - recalcular flood fill
   - elegir siguiente dirección
   - girar si hace falta
   - avanzar una celda

cuando encuentra meta:
-> reconstruir mejor camino
-> pasar a speed-run

speed-run:
-> ejecutar secuencia de movimientos óptima
-> usar encoders + gyro + paredes para máxima precisión

# 29. Mi recomendación final de control

Para tu caso concreto, yo haría esto sí o sí:

encoders AS5600 = verdad principal de distancia
gyro = verdad principal de giro
IR laterales = corrección de centrado
ToF = pared frontal y alineación
acelerómetro = debugging y quizá detección de vibración/impacto, no como fuente principal de posición

Esa combinación es la más sana para tu arquitectura.

# 30. Qué haría en el siguiente paso

Lo más útil ahora es una de estas dos cosas:

convertir este pseudocódigo en una arquitectura de archivos para PlatformIO/Arduino/ESP-IDF, o
bajar este pseudocódigo a una versión más concreta tipo C, con nombres de funciones, structs y módulos.

Te conviene mucho hacer eso antes de cablear todo definitivo.