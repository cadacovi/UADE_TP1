# UADE_TP1 - Robot móvil autónomo con ESP32-S3

# Resumen

Proyecto de robótica simple basado en un carrito diferencial capaz de desplazarse por un mapa local de celdas, detectar obstáculos, replanificar una ruta y ejecutar una acción de descarga mediante servo.

## Objetivo

El objetivo del proyecto es construir un robot móvil que pueda recibir un destino relativo, desplazarse hacia él usando odometría y sensores, esquivar obstáculos dentro de un entorno discretizado en celdas, y activar un mecanismo de descarga al llegar al destino.

## Características principales

- Controlado por ESP32-S3.
- Movimiento diferencial con driver DRV8833.
- Odometría mediante encoders ópticos.
- Corrección de orientación usando IMU MPU6050.
- Detección de obstáculos con sensores ultrasónicos HC-SR04.
- Mapa local temporal basado en celdas.
- Planificación de ruta mediante búsqueda en grilla.
- Replanificación ante obstáculos detectados.
- Control remoto mediante WiFi en modo Access Point.
- Servo para mecanismo de descarga.
- Código modular en archivos `.h` y `.cpp`.

## Componentes utilizados

- ESP32-S3 DevKit.
- Driver de motores DRV8833.
- 2 motores TT DC con ruedas.
- 2 encoders ópticos LM393 con disco ranurado.
- IMU MPU6050 / GY-521.
- 3 sensores ultrasónicos HC-SR04.
- Servo MG90S.
- Buck regulador de 5 V.
- Power bank para alimentación estable del ESP32-S3.
- Portapilas 4xAA para alimentación de motores.
- Resistencias/divisores de tensión para las señales ECHO de los HC-SR04.

## Modelo de navegación

El robot utiliza un mapa local temporal. Cada misión comienza con el robot en la posición lógica `(0, 0)` y con dirección inicial `NORTE`.

El mapa está dividido en celdas de 25 cm x 25 cm, coherente con el tamaño fisico del robot. Cada celda puede estar en uno de los siguientes estados:

- `DESCONOCIDA`
- `LIBRE`
- `OCUPADA`
- `VISITADA`

La posición lógica del robot representa aproximadamente el centro de giro del robot, ubicado cerca del eje de las ruedas motrices.

## Funcionamiento general

1. El robot recibe un destino relativo mediante comando.
2. Inicializa un mapa local para la misión.
3. Lee los sensores ultrasónicos y actualiza el mapa.
4. Planifica una ruta hacia el destino. Se uso el algoritmo BFS (Breadth-First Search).
5. Avanza siguiendo la ruta.
6. Si detecta un obstáculo, marca la celda correspondiente como ocupada.
7. Replanifica la ruta si es necesario.
8. Al llegar al destino, activa el servo de descarga.

## Control de movimiento

El avance se controla usando encoders e IMU.

- Los encoders estiman la distancia recorrida.
- La IMU ayuda a mantener la orientación durante el avance.
- El robot corrige diferencias entre ruedas mediante una ganancia proporcional.
- Los giros se realizan usando el yaw relativo de la IMU.
- Se aplica corrección fina para reducir errores de orientación.

También se implementó una mejora para avanzar varios tramos rectos consecutivos sin detenerse en cada celda, mientras la ruta siga en la misma dirección.

## Comunicación WiFi

El ESP32-S3 crea una red WiFi propia en modo Access Point.

Datos por defecto:

```text
SSID: RobotESP32
Password: 12345678
IP: 192.168.4.1
```

# Dificultades, limitaciones y posibles mejoras

## Dificultades principales durante el desarrollo

Durante el desarrollo del robot se encontraron varias dificultades prácticas relacionadas con la integración entre hardware, sensores, alimentación y control de movimiento.

### Ruido en los encoders

Uno de los problemas más importantes fue lograr lecturas repetibles de los encoders ópticos. Inicialmente, los sensores entregaban una cantidad de pulsos muy superior a la esperada y con picos erráticos. Esto hacía que la odometría fuera poco confiable y que el robot creyera haber avanzado más distancia de la real.

El problema se identificó como una combinación de rebote/ruido en la señal digital del módulo comparador LM393, sensibilidad a falsos contactos y vibraciones mecánicas del disco ranurado. Para resolverlo se implementó un filtrado temporal en la interrupción del encoder, ignorando pulsos que ocurrieran demasiado cerca del pulso anterior. Después de esta mejora, las lecturas se volvieron mucho más estables y repetibles.

### Calibración de distancia

Aunque inicialmente se intentó estimar la distancia usando el número teórico de ranuras del disco y el diámetro de la rueda, en la práctica esa aproximación no resultó suficientemente precisa. Se optó por calibrar empíricamente el valor de `PULSOS_POR_MM`, empujando el robot una distancia conocida y calculando la relación real entre pulsos y milímetros recorridos.

Esta calibración permitió que funciones como `AV200` (avanzar 200mm) terminaran cerca de la distancia objetivo.

### Corrección de avance recto

Durante las pruebas se observó que una rueda podía avanzar ligeramente más que la otra, provocando que el robot se desviara durante trayectos rectos. Para corregir esto se implementó una corrección proporcional basada en la diferencia de distancia medida por ambos encoders.

Además, se usó la IMU para corregir desviaciones de yaw durante el avance. Esto permitió que el robot mantuviera una trayectoria más recta.

### Desorientación por frenado y arranque

El robot podía desviarse levemente al arrancar o detenerse, especialmente por diferencias de fricción, inercia y respuesta de los motores. Para reducir este efecto se agregaron correcciones finas de orientación usando la IMU después de ciertos movimientos, y se ajustaron los valores de PWM para evitar frenadas demasiado bruscas.

### Alimentación insuficiente

Otro problema importante fue la alimentación. Inicialmente se intentó alimentar el sistema completo con pilas AA, pero al activar WiFi y motores se producían caídas de tensión que reiniciaban el ESP32-S3. El problema se agravaba por el uso de protoboard y cables Dupont, que introducen resistencia y falsos contactos.

Para la versión de demostración se optó por alimentar el ESP32-S3 mediante un power bank USB, manteniendo una alimentación separada para motores y compartiendo tierra común. Esto permitió estabilizar el WiFi y el control del robot.

### Integración de sensores ultrasónicos

Los sensores HC-SR04 entregan la señal ECHO a 5 V, mientras que el ESP32-S3 trabaja a 3.3 V. Por seguridad se incorporaron divisores de tensión para adaptar esta señal. También se implementaron dos tipos de lectura: una lectura filtrada para actualizar el mapa y una lectura rápida para detección frontal durante el movimiento. Ambas realizan varias mediciones y toman la mediana para reducir lecturas erráticas; la diferencia es que la lectura rápida usa menos muestras y menor retardo entre ellas para no ralentizar el control de movimiento.

## Limitaciones actuales

La versión actual del robot cumple el objetivo de navegación básica, pero tiene limitaciones propias del modelo implementado y de la construcción física.

### Mapa discreto por celdas

El entorno se representa como una grilla de celdas de 25 cm x 25 cm. Este modelo simplifica la planificación y permite trabajar con rutas discretas, pero tiene dificultades cuando un obstáculo ocupa parcialmente una celda o se encuentra entre dos celdas. En esos casos, el robot puede no representar perfectamente la geometría real del entorno, lo que puede provocar detecciones tardías, replanificaciones incorrectas o incluso contacto físico con el obstáculo. Como consecuencia, la misión puede no completarse correctamente.

### Sensado limitado

El robot utiliza tres sensores ultrasónicos: frontal, izquierdo y derecho. Estos sensores tienen un campo de visión limitado, por lo que no detectan todos los obstáculos posibles, especialmente obstáculos pequeños, diagonales o ubicados fuera de la línea de visión directa.

### Acumulación de error

La posición se estima mediante odometría e IMU. Aunque el sistema funciona bien para trayectos cortos, en recorridos largos puede acumular errores por deslizamiento de ruedas, pequeñas diferencias entre motores, vibraciones o deriva de la IMU.

### Orientación relativa

La IMU permite estimar yaw relativo, pero no entrega una orientación absoluta como una brújula. Por lo tanto, la orientación depende del punto de inicio de la misión y puede derivar lentamente con el tiempo.

### Tamaño máximo del mapa

El mapa local tiene un tamaño limitado por las dimensiones definidas en código. En teoría, el robot podría recibir destinos más lejanos, pero en la práctica está limitado por memoria, seguridad, precisión de odometría y tamaño del entorno de pruebas.

### Alimentación y conexiones

La versión actual utiliza protoboard y cables Dupont, lo cual facilita el prototipado pero no es ideal para un robot móvil. Las vibraciones y los consumos de corriente pueden generar caídas de tensión o falsos contactos.

## Posibles mejoras para una segunda versión

Para una segunda versión del robot, se podrían implementar las siguientes mejoras:

### Alimentación más robusta

Reemplazar las pilas AA por un pack Li-ion o LiPo con BMS, reguladores adecuados y mayor capacidad de corriente. Esto permitiría alimentar de forma más estable al ESP32-S3, motores, sensores y servo sin depender de un power bank externo.

### Mejor cableado y montaje

Migrar el circuito desde protoboard a una placa perforada o PCB, con conectores firmes, cables más cortos y mejor distribución de alimentación. Esto reduciría falsos contactos y caídas de tensión.

### Control PID

Reemplazar las correcciones proporcionales actuales por controladores PID para avance recto y giro. Esto permitiría movimientos más suaves, menor oscilación y mejor precisión.

### Navegación continua

La versión actual usa navegación discreta por celdas. Una mejora importante sería implementar navegación continua no bloqueante, donde el robot actualice sensores, posición y mapa mientras se mueve, sin detenerse en cada celda.

### Mapa probabilístico

En lugar de representar cada celda solamente como libre, ocupada o desconocida, se podría usar un mapa con probabilidades de ocupación. Esto permitiría manejar mejor lecturas inciertas o obstáculos parcialmente detectados.

### Mejor detección de obstáculos

Agregar sensores diagonales, sensores infrarrojos o una mejor estrategia de escaneo permitiría detectar obstáculos que actualmente pueden quedar fuera del campo de visión de los ultrasónicos.

### Interfaz web mejorada

La interfaz WiFi podría mostrar el mapa local, la posición del robot, la ruta planificada, lecturas de sensores y estado de batería en tiempo real.

### Medición de batería

Agregar un divisor resistivo conectado a un pin ADC del ESP32-S3 permitiría medir el voltaje de batería y adaptar el comportamiento del robot según el estado de carga.

### Mejor estructura mecánica

Diseñar un chasis más rígido y compacto, con mejor distribución de peso, ayudaría a mejorar la estabilidad, la tracción y la repetibilidad de los movimientos.