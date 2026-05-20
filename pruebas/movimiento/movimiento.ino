#include "config.h"
#include "motores.h"
#include "encoders.h"
#include "imu.h"
#include "ultrasonidos.h"
#include "control_movimiento.h"

void mostrarAyuda() {
    Serial.println();
    Serial.println("===== PRUEBA CONTROL MOVIMIENTO =====");
    Serial.println("AV100    -> avanzar 100 mm");
    Serial.println("AV200    -> avanzar 200 mm");
    Serial.println("AVCELDA  -> avanzar una celda");
    Serial.println("GDER     -> girar 90 grados derecha");
    Serial.println("GIZQ     -> girar 90 grados izquierda");
    Serial.println("G180     -> girar 180 grados");
    Serial.println("S        -> detener motores");
    Serial.println("E        -> imprimir encoders");
    Serial.println("IMU      -> imprimir IMU");
    Serial.println("U        -> imprimir ultrasonidos");
    Serial.println("YAW0     -> resetear yaw");
    Serial.println("ER       -> resetear encoders");
    Serial.println("AYUDA    -> mostrar ayuda");
    Serial.println("=====================================");
    Serial.println();
}

void inicializarTodo() {
    inicializarMotores();
    inicializarEncoders();

    bool imuOk = inicializarIMU();
    if (!imuOk) {
        Serial.println("[TEST] Advertencia: IMU no inicializada");
    }

    inicializarUltrasonidos();

    detenerMotores();
}

void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    delay(1000);

    Serial.println();
    Serial.println("Iniciando prueba de control de movimiento...");
    Serial.println("Mantener el robot quieto durante la calibracion de IMU.");

    inicializarTodo();

    mostrarAyuda();
}

void loop() {
    actualizarIMU();

    if (!Serial.available()) {
        return;
    }

    String comando = Serial.readStringUntil('\n');
    comando.trim();
    comando.toUpperCase();

    if (comando == "AV100") {
        avanzarDistanciaMm(100.0);
    }

    else if (comando == "AV200") {
        avanzarDistanciaMm(200.0);
    }

    else if (comando == "AVCELDA") {
        avanzarUnaCelda();
    }

    else if (comando == "GDER") {
        girar90Derecha();
    }

    else if (comando == "GIZQ") {
        girar90Izquierda();
    }

    else if (comando == "G180") {
        girar180();
    }

    else if (comando == "S") {
        detenerMotores();
        Serial.println("[TEST] Motores detenidos");
    }

    else if (comando == "E") {
        imprimirEncoders();
    }

    else if (comando == "IMU") {
        imprimirIMU();
    }

    else if (comando == "U") {
        imprimirUltrasonidos();
    }

    else if (comando == "YAW0") {
        resetYaw();
        Serial.println("[TEST] Yaw reseteado");
    }

    else if (comando == "ER") {
        resetEncoders();
        Serial.println("[TEST] Encoders reseteados");
    }

    else if (comando == "AYUDA") {
        mostrarAyuda();
    }

    else {
        Serial.println("[TEST] Comando no reconocido");
    }
}