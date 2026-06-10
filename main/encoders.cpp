#include "encoders.h"

volatile long pulsosIzquierdo = 0; // Volatile = “Esta variable puede cambiar en cualquier momento, incluso fuera del flujo normal del programa.” volatile no protege la variable. Solo obliga al programa a leerla realmente de memoria cada vez, porque puede cambiar inesperadamente.
volatile long pulsosDerecho = 0;

volatile unsigned long ultimoPulsoIzqUs = 0;
volatile unsigned long ultimoPulsoDerUs = 0;

const unsigned long FILTRO_ENCODER_US = 2000; // probar 1000, 2000, 3000

// portMUX_TYPE: Tipo de dato usado para crear el “candado” de sección crítica en ESP32. Explicacion de muxEncoders mas detallada un poco mas abajo.
portMUX_TYPE muxEncoders = portMUX_INITIALIZER_UNLOCKED; //Inicializa el candado en estado libre, es decir, no tomado por nadie.

void IRAM_ATTR encoderIzquierdoISR() {
    // IRAM_ATTR: Le indica al compilador que esa función debe colocarse en memoria interna rápida, no en memoria flash externa.
    // En ESP32 esto es importante para interrupciones. Si el micro está atendiendo una interrupción y justo la flash no está disponible por alguna operación interna, una ISR ubicada en flash podría fallar o tardar demasiado. Por eso se acostumbra marcar las ISR con IRAM_ATTR.

    unsigned long ahora = micros();

    if (ahora - ultimoPulsoIzqUs < FILTRO_ENCODER_US) {
        // Obervacion con respecto al overflow de micros()
        // micros() sí vuelve a cero, pero si comparás tiempos restando ahora - tiempoAnterior, con variables unsigned long, no tenés que preocuparte para intervalos cortos como 2000 µs. Incluso si micros() vuelve a cero justo después de un pulso, el cálculo de ahora - ultimoPulsoIzqUs seguirá siendo correcto mientras el intervalo entre pulsos sea menor a aproximadamente 71 minutos (el tiempo que tarda un unsigned long en overflowear con micros()). En este caso, como el filtro es de solo 2000 µs, no hay riesgo de que el overflow afecte la lógica del filtro.
        return;
    }

    portENTER_CRITICAL_ISR(&muxEncoders);
    pulsosIzquierdo++;
    ultimoPulsoIzqUs = ahora;
    portEXIT_CRITICAL_ISR(&muxEncoders);
}

void IRAM_ATTR encoderDerechoISR() {
    unsigned long ahora = micros();

    if (ahora - ultimoPulsoDerUs < FILTRO_ENCODER_US) {
        return;
    }

    // "muxEncoders" es como un “candado” o mecanismo de exclusión mutua usado por el ESP32/FreeRTOS. Cuando queremos acceder a una variable compartida (en este caso, pulsosIzquierdo o pulsosDerecho), usamos portENTER_CRITICAL_ISR(&muxEncoders) para “cerrar el candado” y asegurarnos de que ningún otro código pueda interrumpirnos justo en medio de la lectura o escritura de esa variable. Luego, después de terminar de modificar la variable, usamos portEXIT_CRITICAL_ISR(&muxEncoders) para “abrir el candado” y permitir que otros códigos puedan acceder a esa variable nuevamente. Esto es crucial para evitar condiciones de carrera y garantizar la integridad de los datos compartidos entre el código principal y las interrupciones.
    
    portENTER_CRITICAL_ISR(&muxEncoders); // entra en seccion critica
    pulsosDerecho++;
    ultimoPulsoDerUs = ahora;
    portEXIT_CRITICAL_ISR(&muxEncoders); // sale de seccion critica
    // Seccion critica: “Durante estas líneas, no me interrumpas justo mientras estoy leyendo esta variable compartida.”
    // ¿Por qué importa? -> en microcontroladores no siempre una lectura/escritura de una variable grande ocurre en una sola operación atómica. Además, en ESP32 tenés doble núcleo y FreeRTOS, por lo que pueden existir más fuentes de concurrencia que en un Arduino UNO clásico.
    // "_ISR" al final es porque se esta entrando en seccion critica desde una interrupcion, y eso tiene algunas diferencias técnicas (ej: no se pueden usar ciertas funciones de FreeRTOS que podrían bloquear, porque no se puede bloquear dentro de una interrupción). En este caso, como solo estamos incrementando un contador y actualizando un timestamp, no hay riesgo de bloqueo, por lo que podemos usar portENTER_CRITICAL_ISR sin problemas.
}

void inicializarEncoders() {
    pinMode(ENCODER_LEFT, INPUT_PULLUP);
    pinMode(ENCODER_RIGHT, INPUT_PULLUP);

    resetEncoders();

    attachInterrupt(digitalPinToInterrupt(ENCODER_LEFT), encoderIzquierdoISR, FALLING);
    // “Cuando el pin del encoder izquierdo tenga un flanco descendente (HIGH -> LOW), pausá momentáneamente lo que estés haciendo y ejecutá encoderIzquierdoISR().”
    attachInterrupt(digitalPinToInterrupt(ENCODER_RIGHT), encoderDerechoISR, FALLING);

    Serial.println("[ENCODERS] Inicializados");
}

void resetEncoders() {
    portENTER_CRITICAL(&muxEncoders);
    pulsosIzquierdo = 0;
    pulsosDerecho = 0;
    ultimoPulsoIzqUs = 0;
    ultimoPulsoDerUs = 0;
    portEXIT_CRITICAL(&muxEncoders);
}

long obtenerPulsosIzquierdo() {
    long copia;

    portENTER_CRITICAL(&muxEncoders);
    copia = pulsosIzquierdo;
    portEXIT_CRITICAL(&muxEncoders);

    return copia;
}

long obtenerPulsosDerecho() {
    long copia;

    portENTER_CRITICAL(&muxEncoders);
    copia = pulsosDerecho;
    portEXIT_CRITICAL(&muxEncoders);

    return copia;
}

float obtenerDistanciaIzquierdaMm() {
    return obtenerPulsosIzquierdo() / PULSOS_POR_MM;
}

float obtenerDistanciaDerechaMm() {
    return obtenerPulsosDerecho() / PULSOS_POR_MM;
}

float obtenerDistanciaPromedioMm() {
    float izquierda = obtenerDistanciaIzquierdaMm();
    float derecha = obtenerDistanciaDerechaMm();

    return (izquierda + derecha) / 2.0;
}

float obtenerErrorRuedasMm() {
    return obtenerDistanciaIzquierdaMm() - obtenerDistanciaDerechaMm();
}

void imprimirEncoders() {
    Serial.println();
    Serial.println("===== ENCODERS =====");

    Serial.print("Pulsos izquierdo: ");
    Serial.println(obtenerPulsosIzquierdo());

    Serial.print("Pulsos derecho: ");
    Serial.println(obtenerPulsosDerecho());

    Serial.print("Distancia izquierda mm: ");
    Serial.println(obtenerDistanciaIzquierdaMm());

    Serial.print("Distancia derecha mm: ");
    Serial.println(obtenerDistanciaDerechaMm());

    Serial.print("Distancia promedio mm: ");
    Serial.println(obtenerDistanciaPromedioMm());

    Serial.print("Error ruedas mm: ");
    Serial.println(obtenerErrorRuedasMm());

    Serial.println("====================");
    Serial.println();
}