## Resumen de módulos de programación

El código del robot se organizó de forma modular, separando sensores, actuadores, navegación, planificación, comunicación y control general. Esto permitió probar cada parte por separado y luego integrarlas dentro de una lógica principal de misión.

### Configuración y tipos generales

Los archivos `config.h` y `tipos.h` contienen la base común del proyecto. En `config.h` se definen pines, constantes físicas, tamaño de celda, umbrales de obstáculos, valores de PWM, ganancias de control, parámetros de WiFi y valores de calibración. En `tipos.h` se definen estructuras y enumeraciones usadas por todo el sistema, como `Celda`, `Ruta`, `EstadoRobot`, `Direccion`, `EstadoCelda`, `ResultadoAvance` y `ResultadoGiro`.

Estos archivos permiten que todos los módulos compartan la misma configuración y representación lógica del robot.

### Módulos de hardware

Los módulos `motores`, `encoders`, `imu`, `ultrasonidos` y `servo_carga` encapsulan el manejo directo de los componentes físicos.

El módulo `motores` controla el driver DRV8833 mediante señales PWM. Usa valores positivos o negativos para indicar sentido de giro, lo que simplifica el manejo de avance, retroceso y giro diferencial.

El módulo `encoders` cuenta los pulsos de los encoders ópticos mediante interrupciones. Esto permite registrar pulsos aunque el programa esté realizando otras tareas. Para reducir errores por ruido, rebote o vibraciones, se implementó un filtro temporal dentro de la interrupción, descartando pulsos demasiado cercanos entre sí. También se usan variables `volatile` y secciones críticas para proteger los contadores compartidos entre las interrupciones y el programa principal.

El módulo `imu` gestiona el MPU6050 por I2C. Se utiliza principalmente el giroscopio en el eje Z para calcular un yaw relativo, integrando la velocidad angular en el tiempo. Este yaw se usa para controlar giros y corregir desviaciones durante el avance recto.

El módulo `ultrasonidos` controla los tres sensores HC-SR04. Para mejorar la estabilidad de las mediciones, toma varias muestras y aplica un filtrado por mediana. También se implementó una lectura frontal rápida, con menos muestras, para detectar obstáculos durante el movimiento sin ralentizar demasiado el control.

El módulo `servo_carga` controla el servo MG90S del mecanismo de descarga. Permite abrir, cerrar y realizar una secuencia completa de descarga mediante una función de alto nivel.

### Control de movimiento

El módulo `control_movimiento` convierte órdenes simples, como avanzar una distancia o girar un ángulo, en movimientos físicos controlados. Para esto combina motores, encoders, IMU y sensores ultrasónicos.

Durante el avance, el robot calcula la distancia recorrida con los encoders y corrige la trayectoria usando dos errores: la diferencia entre ruedas y el error de yaw medido por la IMU. Además, reduce el PWM cerca del final del recorrido para evitar sobrepasarse y revisa periódicamente el sensor frontal para detenerse si aparece un obstáculo.

Durante los giros, el módulo usa el yaw relativo de la IMU para detener el movimiento al alcanzar el ángulo objetivo. También aplica límites de PWM y una reducción de velocidad al final del giro para mejorar la precisión.

### Mapa local y planificación

El módulo `mapa_local` representa el entorno como una grilla de celdas. Cada celda puede estar como desconocida, libre, ocupada o visitada. El mapa es local y temporal, ya que se reinicia al comenzar cada misión y toma como origen la posición inicial del robot.

El módulo `planificador` calcula rutas usando BFS, o búsqueda en anchura. Este algoritmo es adecuado porque el entorno se modela como una grilla de costo uniforme, donde cada paso entre celdas tiene el mismo costo. BFS permite encontrar una ruta válida con la menor cantidad de pasos, evitando celdas marcadas como ocupadas.

### Módulo principal `robot`

El módulo `robot` es el controlador principal del sistema. Integra sensores, actuadores, mapa, planificación, movimiento, descarga y comunicación con los demás módulos. Mantiene la posición lógica del robot, su dirección, el destino, la ruta actual, el estado de la misión y banderas como `misionActiva`, `cargaEntregada` y `regresandoAlOrigen`.

Una parte central de este módulo es la máquina de estados. La función `actualizar()` se ejecuta continuamente y llama al manejador correspondiente según el estado actual del robot. Los estados principales son reposo, planificación de ruta, navegación, evasión de obstáculo, descarga, misión completada y error.

Entre las funciones más importantes del módulo se encuentran:

* `crearNuevaMision()`: reinicia la posición lógica, el mapa y la ruta, y establece un nuevo destino.
* `actualizarMapaLocalConSensores()`: traduce las lecturas de los ultrasónicos a celdas del mapa, marcándolas como libres u ocupadas según corresponda.
* `orientarHacia()`: gira el robot hasta que su dirección lógica coincida con la dirección requerida por la ruta.
* `manejarPlanificandoRuta()`: actualiza el mapa y llama al planificador BFS.
* `manejarNavegando()`: ejecuta la ruta planificada y decide si debe avanzar, replanificar, descargar o finalizar.
* `avanzarDerecho()`: permite avanzar varias celdas consecutivas en línea recta, en lugar de detenerse en cada celda. Si se detecta un obstáculo durante el tramo, estima cuántas celdas completas se avanzaron, retrocede hasta una posición segura, actualiza la posición lógica y marca la celda frontal como ocupada.
* `manejarDescargando()`: activa el servo de descarga y luego cambia el destino al origen para que el robot vuelva automáticamente.

Este módulo es el más amplio porque concentra la lógica de alto nivel de la misión, pero no reemplaza a los módulos específicos. Su función principal es coordinar el comportamiento general del robot.

### Comunicación

El módulo `comandos` permite controlar y depurar el robot desde el monitor serie. Incluye comandos para iniciar misiones, detener el robot, consultar sensores, imprimir el mapa, imprimir la ruta y probar actuadores.

El módulo `wifi_comandos` implementa una interfaz web. El ESP32-S3 crea una red WiFi propia en modo Access Point y sirve una página de control con botones para movimiento manual, misiones rápidas, sensores, descarga y STOP. Esto facilita la demostración del proyecto desde un celular o computadora sin depender solamente del monitor serie.

### Programa principal

El archivo `main.ino` es el punto de entrada del programa. Se mantiene intencionalmente corto: en `setup()` inicializa la comunicación, el robot y el servidor WiFi; en `loop()` lee comandos serie, atiende comandos WiFi y actualiza la lógica principal del robot.

Esta estructura permite que el programa principal sea simple, mientras la complejidad queda distribuida en módulos especializados.
