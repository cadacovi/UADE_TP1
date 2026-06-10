## Módulo `encoders`

### Descripción general

El módulo `encoders` se encarga de leer los encoders ópticos asociados a las ruedas izquierda y derecha del robot. Su función principal es contar los pulsos generados por cada encoder y convertirlos en una estimación de distancia recorrida.

Este módulo es fundamental para la odometría del robot, ya que permite estimar cuánto avanzó cada rueda, calcular la distancia promedio recorrida y detectar diferencias entre ambas ruedas. Esa información luego es utilizada por el módulo de control de movimiento para avanzar distancias determinadas y corregir desvíos durante el movimiento.

### Funcionamiento general

Cada encoder genera una señal digital que cambia cuando el disco ranurado interrumpe o permite el paso de luz en el sensor óptico. El ESP32-S3 detecta esos cambios mediante interrupciones externas configuradas sobre los pines de los encoders.

En este caso, las interrupciones se configuran en modo `FALLING`, es decir, se cuenta un pulso cuando la señal pasa de nivel alto a nivel bajo.

Para evitar conteos erráticos por ruido, vibraciones o rebotes en la señal del módulo LM393, se implementa un filtro temporal dentro de la interrupción. Si dos pulsos ocurren demasiado cerca en el tiempo, el segundo se ignora.

### Variables principales

* `pulsosIzquierdo`: contador de pulsos del encoder izquierdo.
* `pulsosDerecho`: contador de pulsos del encoder derecho.
* `ultimoPulsoIzqUs`: instante, en microsegundos, del último pulso válido del encoder izquierdo.
* `ultimoPulsoDerUs`: instante, en microsegundos, del último pulso válido del encoder derecho.
* `FILTRO_ENCODER_US`: tiempo mínimo permitido entre dos pulsos consecutivos válidos.
* `muxEncoders`: mecanismo de exclusión mutua usado para proteger el acceso a variables compartidas entre el programa principal y las interrupciones.

Las variables de conteo se declaran como `volatile` porque son modificadas dentro de rutinas de interrupción y también leídas desde el programa principal.

### Funciones principales

#### `inicializarEncoders()`

Configura los pines de los encoders como entradas con resistencia pull-up interna mediante `INPUT_PULLUP`.

Luego reinicia los contadores llamando a `resetEncoders()` y configura las interrupciones externas:

* Encoder izquierdo: interrupción asociada al pin `ENCODER_LEFT`.
* Encoder derecho: interrupción asociada al pin `ENCODER_RIGHT`.
* Modo de disparo: `FALLING`.

Finalmente, imprime por monitor serie un mensaje indicando que el módulo fue inicializado.

#### `encoderIzquierdoISR()`

Es la rutina de interrupción del encoder izquierdo.

Cada vez que se detecta un flanco descendente, se obtiene el tiempo actual con `micros()`. Si el tiempo transcurrido desde el último pulso válido es menor que `FILTRO_ENCODER_US`, el pulso se descarta. Si supera ese tiempo mínimo, se incrementa el contador `pulsosIzquierdo`.

Esta estrategia permite reducir falsos pulsos producidos por ruido eléctrico, rebotes o vibraciones mecánicas.

#### `encoderDerechoISR()`

Es la rutina de interrupción del encoder derecho. Tiene el mismo funcionamiento que la interrupción del encoder izquierdo, pero actualiza el contador `pulsosDerecho`.

#### `resetEncoders()`

Reinicia los contadores de ambos encoders y los tiempos del último pulso válido.

Esta función se utiliza antes de iniciar un movimiento, de manera que las mediciones de distancia comiencen desde cero para ese avance o giro específico.

#### `obtenerPulsosIzquierdo()`

Devuelve una copia segura del contador de pulsos del encoder izquierdo.

El acceso se realiza dentro de una sección crítica para evitar leer la variable mientras está siendo modificada por una interrupción.

#### `obtenerPulsosDerecho()`

Devuelve una copia segura del contador de pulsos del encoder derecho, también protegida mediante sección crítica.

#### `obtenerDistanciaIzquierdaMm()`

Convierte los pulsos del encoder izquierdo a distancia recorrida en milímetros usando la constante calibrada `PULSOS_POR_MM`.

La relación utilizada es:

```cpp
distancia = pulsos / PULSOS_POR_MM;
```

#### `obtenerDistanciaDerechaMm()`

Convierte los pulsos del encoder derecho a distancia recorrida en milímetros, usando la misma relación de calibración.

#### `obtenerDistanciaPromedioMm()`

Calcula la distancia promedio recorrida por el robot a partir de las distancias medidas en ambas ruedas.

Esta función es especialmente útil para estimar cuánto avanzó el centro del robot durante un movimiento recto.

#### `obtenerErrorRuedasMm()`

Calcula la diferencia de distancia entre la rueda izquierda y la rueda derecha:

```cpp
error = distanciaIzquierda - distanciaDerecha;
```

Este error permite saber si una rueda avanzó más que la otra. El módulo de control de movimiento puede usar este dato para corregir la trayectoria durante el avance.

#### `imprimirEncoders()`

Imprime por monitor serie un resumen del estado actual de los encoders:

* Pulsos del encoder izquierdo.
* Pulsos del encoder derecho.
* Distancia estimada de la rueda izquierda.
* Distancia estimada de la rueda derecha.
* Distancia promedio.
* Error entre ruedas.

Esta función es útil para pruebas, calibración y diagnóstico.

### Constantes relacionadas

Este módulo utiliza varias constantes definidas en `config.h`:

* `ENCODER_LEFT`: pin conectado al encoder izquierdo.
* `ENCODER_RIGHT`: pin conectado al encoder derecho.
* `PULSOS_POR_MM`: relación calibrada entre pulsos y distancia recorrida.

Además, define internamente:

* `FILTRO_ENCODER_US`: tiempo mínimo, en microsegundos, que debe pasar entre dos pulsos consecutivos para que el segundo sea considerado válido.

### Relación con otros módulos

El módulo `encoders` es utilizado principalmente por `control_movimiento`.

Durante un avance, `control_movimiento` puede consultar:

* La distancia promedio recorrida.
* La distancia de cada rueda.
* El error entre ruedas.

Con esa información se puede determinar cuándo detener el avance, corregir diferencias entre motores y detectar posibles problemas de movimiento.

También puede ser usado desde `comandos` o `wifi_comandos` para imprimir valores de diagnóstico durante pruebas del robot.

### Comentarios de diseño

Una decisión importante de este módulo fue contar pulsos mediante interrupciones en lugar de hacer lecturas periódicas en el `loop`. Esto permite no perder pulsos aunque el programa esté ejecutando otras tareas.

También se implementó un filtro temporal directamente dentro de la interrupción. Esta solución fue necesaria porque los módulos ópticos con LM393 pueden entregar señales ruidosas o pulsos falsos debido a vibraciones, falsos contactos o irregularidades mecánicas del disco ranurado. Sin embargo, el filtrado está hecho a nivel lógico dentro de la ISR, no a nivel de hardware antes de disparar la ISR. Eso sirve para no arruinar la odometría, pero no protege del todo a la CPU contra una tormenta de interrupciones. Las instrucciones en interrupciones son cortas, esto puede ayudar a minimizar el problema fisico.

Otra decisión relevante fue proteger las variables compartidas mediante secciones críticas. Como los contadores de pulsos son modificados por interrupciones y leídos desde el programa principal, este mecanismo evita lecturas inconsistentes.

En conjunto, este módulo permite obtener una medición de distancia suficientemente estable para controlar el avance por celdas y corregir diferencias entre ambas ruedas.
