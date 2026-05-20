# Pinout propuesto - carrito robot con ESP32-S3

## Componentes asumidos
- ESP32-S3 DevKit
- DRV8833 breakout
- GY-521 / MPU-6050 (I2C)
- 3 x HC-SR04
- 2 x encoder FC-03 con disco ranurado
- Servo MG90S
- 1 x 74HC4050 para adaptar los ECHO de los HC-SR04 a 3.3 V
- 1 x buck-boost de 5 V >= 2 A

## Alimentacion
- Portapilas 4xAA -> interruptor SW1
- Desde SW1:
  - rama 1 -> VM del DRV8833 (VBAT)
  - rama 2 -> buck-boost 5 V
- Salida del buck-boost 5 V -> ESP32 (pin 5V/VBUS), servo y 3 HC-SR04
- Salida 3V3 del ESP32 -> GY-521, FC-03 y 74HC4050
- Todas las masas unidas

## I2C
- GPIO8 -> SDA del GY-521 / MPU-6050
- GPIO9 -> SCL del GY-521 / MPU-6050

## Ultrasonicos
- Frontal:
  - GPIO4 -> TRIG
  - ECHO -> 74HC4050 canal 1 -> GPIO5
- Izquierdo:
  - GPIO6 -> TRIG
  - ECHO -> 74HC4050 canal 2 -> GPIO7
- Derecho:
  - GPIO15 -> TRIG
  - ECHO -> 74HC4050 canal 3 -> GPIO16

## Encoders
- Encoder izquierdo FC-03:
  - DO -> GPIO17
- Encoder derecho FC-03:
  - DO -> GPIO18
- Alimentacion de ambos FC-03 a 3V3

## Driver de motores DRV8833
- VM -> VBAT
- VCC -> 3V3
- GND -> GND comun
- nSLEEP -> GPIO10 con pull-up de 10k a 3V3
- AIN1 -> GPIO11
- AIN2 -> GPIO12
- BIN1 -> GPIO13
- BIN2 -> GPIO14
- AOUT1/AOUT2 -> motor izquierdo
- BOUT1/BOUT2 -> motor derecho

## Servo MG90S
- Senal -> GPIO21
- Rojo -> +5V regulado
- Marron/negro -> GND

## Capacitores recomendados
- 470 uF entre +5V y GND cerca del servo
- 470 uF entre VBAT y GND cerca del DRV8833
- 100 nF en cada modulo

## Observaciones
- Los GPIO se pueden remapear si tu placa no expone alguno.
- Los pines ECHO de HC-SR04 no deben entrar directo al ESP32-S3.
- El servo no debe alimentarse desde 3V3.
