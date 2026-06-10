## Módulo `control_movimiento`

### Descripción general

El módulo `control_movimiento` se encarga de ejecutar los movimientos básicos del robot de forma controlada. A diferencia del módulo `motores`, que solamente aplica PWM a cada motor, este módulo utiliza sensores para cerrar el lazo de control y lograr movimientos más precisos.

Sus principales responsabilidades son:

* Avanzar una distancia determinada.
* Retroceder una distancia determinada.
* Avanzar una celda completa del mapa.
* Girar un ángulo específico.
* Realizar giros de 90° a derecha o izquierda.
* Realizar giros de 180°.
* Detectar obstáculos durante el avance.
* Corregir la trayectoria usando encoders e IMU.

Este módulo actúa como una capa intermedia entre los actuadores de bajo nivel y la lógica de navegación del robot.

### Relación con otros módulos

`control_movimiento` utiliza varios módulos del sistema:

* `motores`: para aplicar PWM a cada motor mediante `setMotores()` y detener el robot con `detenerMotores()`.
* `encoders`: para medir distancia recorrida, error entre ruedas y reiniciar conteos antes de cada movimiento.
* `imu`: para actualizar y consultar el yaw relativo del robot.
* `ultrasonidos`: para detectar obstáculos frontales durante el avance.
* `tipos`: para devolver estructuras como `ResultadoAvance` y `ResultadoGiro`.

Gracias a esta integración, el módulo puede transformar órdenes simples, como “avanzar 250 mm” o “girar 90°”, en movimientos reales del robot con corrección y verificación.

### Función auxiliar: `limitarPWMControl(float pwm, int pwmMin, int pwmMax)`

Esta función interna limita el valor PWM calculado por el control.

Su comportamiento es:

* Si el PWM supera el máximo permitido, lo limita a `pwmMax`.
* Si el PWM es menor que el máximo negativo permitido, lo limita a `-pwmMax`.
* Si el PWM es positivo pero demasiado bajo, lo eleva a `pwmMin`.
* Si el PWM es negativo pero demasiado bajo en magnitud, lo limita a `-pwmMin`.

Esto evita dos problemas:

1. Enviar un PWM excesivo a los motores.
2. Enviar un PWM demasiado pequeño que no sea suficiente para vencer la fricción estática del robot.

La función se utiliza tanto durante el avance como durante los giros.

### Avance controlado

#### `avanzarDistanciaMm(float distanciaObjetivoMm)`

Es una función simplificada que solicita avanzar una distancia determinada y devuelve solamente si el movimiento fue exitoso o no.

Internamente llama a:

```cpp id="06q06o"
ResultadoAvance r = avanzarDistanciaResultado(distanciaObjetivoMm);
return r.exito;
```

Se usa cuando solo interesa saber si el avance se completó, sin analizar detalles adicionales.

#### `avanzarDistanciaResultado(float distanciaObjetivoMm)`

Es la función principal de avance controlado.

Devuelve una estructura `ResultadoAvance`, que permite informar:

* Si el avance fue exitoso.
* Si hubo timeout.
* Si se detectó un obstáculo.
* Qué distancia se alcanzó a recorrer.

Esta función permite avanzar tanto hacia adelante como hacia atrás. Si la distancia objetivo es negativa, el robot interpreta el movimiento como reversa.

### Secuencia de funcionamiento del avance

El avance controlado sigue los siguientes pasos generales:

1. Inicializa el resultado del movimiento.
2. Reinicia los encoders.
3. Guarda el yaw actual como referencia.
4. Define el tiempo inicial para controlar timeout.
5. Determina si el movimiento es en reversa.
6. Entra en un ciclo de control.
7. Actualiza la IMU.
8. Calcula la distancia recorrida mediante encoders.
9. Verifica si se alcanzó la distancia objetivo.
10. Verifica si se superó el tiempo máximo permitido.
11. Reduce el PWM al acercarse al objetivo.
12. Si avanza hacia adelante, verifica obstáculos frontales periódicamente.
13. Calcula corrección usando error entre ruedas y error de yaw.
14. Aplica PWM corregido a los motores.
15. Al terminar, detiene motores y realiza una corrección final de yaw si es necesario.

### Corrección durante el avance

Durante el avance se combinan dos fuentes de error:

#### Error entre ruedas

Se obtiene mediante:

```cpp id="0mb3e8"
float errorRuedas = obtenerErrorRuedasMm();
```

Este error indica si una rueda avanzó más que la otra. Si existe diferencia entre ruedas, el robot puede desviarse hacia un lado.

#### Error de yaw

Se calcula comparando el yaw objetivo con el yaw actual:

```cpp id="q5qm7c"
float errorYaw = yawObjetivo - obtenerYawRelativo();
```

Este error indica si el robot se desvió angularmente respecto a la orientación que tenía al iniciar el avance.

#### Corrección total

Ambos errores se combinan mediante una corrección proporcional:

```cpp id="185hch"
float correccion = (KP_ENCODER_AVANCE * errorRuedas) +
                   (KP_YAW_AVANCE * errorYaw);
```

Luego se modifica el PWM de cada rueda:

```cpp id="b9ydz3"
pwmIzquierdo = pwmBase - correccion;
pwmDerecho   = pwmBase + correccion;
```

De esta forma, si una rueda avanza más o si el robot gira levemente, el sistema modifica la potencia de los motores para intentar mantener una trayectoria recta.

### Detección de obstáculos durante el avance

Mientras el robot avanza hacia adelante, se realiza una lectura periódica del sensor ultrasónico frontal:

```cpp id="b64twf"
if (millis() - ultimaLecturaUS >= 80 && !reversa) {
    ultimaLecturaUS = millis();

    if (leerFrontalFiltradoRapidoMm() <= DIST_OBSTACULO_FRENTE_MM) {
        resultado.obstaculoDetectado = true;
        resultado.exito = false;
        break;
    }
}
```

La lectura se realiza cada 80 ms aproximadamente y solo durante el avance hacia adelante, no en reversa.

Si se detecta un obstáculo a una distancia menor o igual que `DIST_OBSTACULO_FRENTE_MM`, el avance se interrumpe. Esto permite que el módulo `robot` pueda marcar la celda correspondiente como ocupada y replanificar la ruta.

### Frenado y corrección final

Cuando el robot se acerca a la distancia objetivo, reduce el PWM base usando `PWM_FINAL_AVANCE`. Esto ayuda a evitar que el robot se pase demasiado por inercia.

Luego de detenerse, actualiza la IMU y calcula el error final de yaw. Si el error es mayor que `TOLERANCIA_CORRECCION_YAW` pero menor que `MAX_CORRECCION_YAW`, ejecuta una corrección angular:

```cpp id="g5kvh7"
girarAngulo(-errorFinalYaw);
```

Esta corrección busca que el robot termine el avance con una orientación más cercana a la inicial.

### `avanzarUnaCelda()`

Esta función permite avanzar una celda completa del mapa local:

```cpp id="2f3rur"
return avanzarDistanciaMm(TAM_CELDA_MM);
```

Como el mapa está discretizado en celdas de tamaño `TAM_CELDA_MM`, esta función es usada por la navegación para desplazarse entre celdas vecinas.

### Giro controlado

#### `girarAngulo(float grados)`

Es una función simplificada para girar una cantidad determinada de grados y devolver únicamente si el giro fue exitoso.

Internamente llama a:

```cpp id="a45ndw"
ResultadoGiro r = girarAnguloResultado(grados);
return r.exito;
```

#### `girarAnguloResultado(float grados)`

Es la función principal de giro controlado.

Permite girar un ángulo positivo o negativo:

* Grados positivos: giro hacia la derecha.
* Grados negativos: giro hacia la izquierda.

Devuelve una estructura `ResultadoGiro`, que informa:

* Si el giro fue exitoso.
* Si hubo timeout.
* El error angular final.

### Secuencia de funcionamiento del giro

El giro controlado sigue estos pasos:

1. Inicializa el resultado.
2. Imprime el ángulo objetivo.
3. Reinicia el yaw relativo.
4. Determina el sentido de giro.
5. Toma el valor absoluto del ángulo objetivo.
6. Inicia un ciclo de control.
7. Actualiza la IMU.
8. Calcula el yaw actual.
9. Calcula el error angular restante.
10. Verifica si el error está dentro de la tolerancia.
11. Verifica si se superó el timeout.
12. Calcula un PWM proporcional al error.
13. Limita el PWM entre valores mínimos y máximos.
14. Reduce la potencia al final del giro para evitar sobrepasarse.
15. Aplica PWM opuesto a los motores para girar sobre su eje.
16. Detiene motores al finalizar.
17. Actualiza el error final y devuelve el resultado.

### Control proporcional del giro

Durante el giro se calcula:

```cpp id="ol59zw"
float pwm = KP_GIRO * error;
```

Cuanto mayor es el error angular restante, mayor es el PWM aplicado. Luego ese valor se limita con `limitarPWMControl()` para mantenerlo dentro de un rango útil.

Al acercarse al objetivo, el código aplica `PWM_FINAL_GIRO` para suavizar el final del giro y reducir el sobrepaso.

### Funciones de giro predefinidas

El módulo incluye funciones auxiliares para giros comunes:

* `girar90Derecha()`: gira 90° hacia la derecha.
* `girar90Izquierda()`: gira 90° hacia la izquierda.
* `girar180()`: gira 180°.

Estas funciones simplifican la lógica de navegación, ya que el módulo `robot` puede pedir giros típicos sin especificar el ángulo cada vez.

### Constantes relacionadas

Este módulo utiliza numerosas constantes definidas en `config.h`:

#### Avance

* `TAM_CELDA_MM`: tamaño de una celda.
* `PWM_BASE_AVANCE`: PWM base durante el avance.
* `PWM_FINAL_AVANCE`: PWM usado cerca del final del avance.
* `PWM_AVANCE_MIN`: PWM mínimo útil durante avance.
* `PWM_AVANCE_MAX`: PWM máximo permitido durante avance.
* `TOLERANCIA_DISTANCIA_MM`: tolerancia de distancia para considerar completado un avance.
* `DISTANCIA_FRENADO_MM`: distancia restante a partir de la cual se reduce velocidad.
* `KP_ENCODER_AVANCE`: ganancia proporcional para corregir diferencia entre ruedas.
* `KP_YAW_AVANCE`: ganancia proporcional para corregir error de orientación.
* `TIMEOUT_AVANCE_CELDA_MS`: tiempo máximo permitido para completar un avance.
* `DIST_OBSTACULO_FRENTE_MM`: umbral de detección frontal.

#### Giro

* `PWM_BASE_GIRO`: referencia general de PWM para giros.
* `PWM_FINAL_GIRO`: PWM usado al final del giro.
* `PWM_GIRO_MIN`: PWM mínimo útil durante el giro.
* `PWM_GIRO_MAX`: PWM máximo permitido durante el giro.
* `KP_GIRO`: ganancia proporcional del giro.
* `TOLERANCIA_GIRO_GRADOS`: tolerancia angular aceptada.
* `TIMEOUT_GIRO_MS`: tiempo máximo permitido para completar un giro.
* `TOLERANCIA_CORRECCION_YAW`: tolerancia para corrección final.
* `MAX_CORRECCION_YAW`: límite máximo para aplicar corrección final de yaw.

### Comentarios de diseño

Una decisión importante fue separar el control directo de motores del control de movimiento. El módulo `motores` solo recibe valores PWM, mientras que `control_movimiento` decide qué PWM aplicar en función de encoders, IMU, distancia objetivo y sensores ultrasónicos.

El avance utiliza una corrección proporcional combinada: una parte basada en la diferencia entre ruedas y otra basada en el error de yaw. Esto permite compensar tanto diferencias mecánicas entre motores como desviaciones de orientación detectadas por la IMU.

El giro también utiliza control proporcional, tomando como referencia el yaw relativo de la IMU. Esto permite realizar giros más controlados que si se usaran únicamente tiempos fijos de motor.

Además, se incorporaron mecanismos de seguridad como timeout y detección frontal de obstáculos. Estos mecanismos evitan que el robot quede intentando moverse indefinidamente o que avance hacia un obstáculo detectado.

En general, este módulo cumple el rol de convertir órdenes de alto nivel, como avanzar una celda o girar 90°, en acciones físicas corregidas mediante sensores.
