#include "comandos.h"
#include "motores.h"
#include "encoders.h"
#include "imu.h"
#include "ultrasonidos.h"
#include "servo_carga.h"
#include "control_movimiento.h"

void iniciarComunicacion() {
    Serial.begin(SERIAL_BAUDRATE);
    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println(" Robot Proyecto - ESP32-S3");
    Serial.println(" Comunicacion Serial iniciada");
    Serial.println("================================");
    Serial.println();

    mostrarAyuda();
}

void reportar(const char* mensaje) {
    Serial.print("[ROBOT] ");
    Serial.println(mensaje);
}

void reportar(String mensaje) {
    Serial.print("[ROBOT] ");
    Serial.println(mensaje);
}

void mostrarAyuda() {
    Serial.println("Comandos disponibles:");
    Serial.println();
    Serial.println("AYUDA       -> mostrar esta ayuda");
    Serial.println("ESTADO      -> imprimir estado actual");
    Serial.println("G x y       -> nueva mision en celdas locales");
    Serial.println("GC x y      -> nueva mision en centimetros");
    Serial.println("S           -> detener robot");
    Serial.println("D           -> descarga manual");
    Serial.println("RESET       -> reiniciar estado logico del robot");
    Serial.println();
    Serial.println("Sistema de coordenadas local:");
    Serial.println("x positivo = derecha");
    Serial.println("x negativo = izquierda");
    Serial.println("y positivo = adelante");
    Serial.println("y negativo = atras");
    Serial.println();
    Serial.println("Ejemplos:");
    Serial.println("G 0 3       -> avanzar 3 celdas");
    Serial.println("G 2 3       -> derecha 2 celdas y adelante 3");
    Serial.println("GC 40 60    -> derecha 40 cm y adelante 60 cm");
    Serial.println("F           -> prueba avanzar");
    Serial.println("B           -> prueba retroceder");
    Serial.println("L           -> prueba girar izquierda");
    Serial.println("R           -> prueba girar derecha");
    Serial.println("E           -> imprimir encoders");
    Serial.println("ER          -> resetear encoders");
    Serial.println("IMU         -> imprimir datos IMU");
    Serial.println("YAW0        -> resetear yaw relativo");
    Serial.println("U           -> imprimir ultrasonidos");
    Serial.println("SERVO       -> imprimir estado del servo");
    Serial.println("ABRIR       -> abrir carga");
    Serial.println("CERRAR      -> cerrar carga");
    Serial.println("AVCELDA     -> avanzar una celda con control");
    Serial.println("GDER        -> girar 90 grados derecha");
    Serial.println("GIZQ        -> girar 90 grados izquierda");
    Serial.println("G180        -> girar 180 grados");
    Serial.println("MAPA        -> imprimir mapa local");
    Serial.println("SENSAR      -> actualizar mapa con ultrasonidos");
    Serial.println("RUTA        -> imprimir ruta actual");
    Serial.println();
}

bool extraerDosEnteros(String comando, int &a, int &b) {
    comando.trim();

    int espacio1 = comando.indexOf(' ');
    if (espacio1 < 0) {
        return false;
    }

    int espacio2 = comando.indexOf(' ', espacio1 + 1);
    if (espacio2 < 0) {
        return false;
    }

    String parteA = comando.substring(espacio1 + 1, espacio2);
    String parteB = comando.substring(espacio2 + 1);

    parteA.trim();
    parteB.trim();

    if (parteA.length() == 0 || parteB.length() == 0) {
        return false;
    }

    a = parteA.toInt();
    b = parteB.toInt();

    return true;
}

void procesarComandoGC(String comando, Robot &robot) {
    int xCm = 0;
    int yCm = 0;

    if (!extraerDosEnteros(comando, xCm, yCm)) {
        reportar("Formato invalido. Usa: GC x_cm y_cm");
        return;
    }

    Celda destino;

    destino.x = (xCm * 10) / TAM_CELDA_MM;
    destino.y = (yCm * 10) / TAM_CELDA_MM;

    Serial.print("[CMD] Destino en cm recibido: x=");
    Serial.print(xCm);
    Serial.print(" cm, y=");
    Serial.print(yCm);
    Serial.println(" cm");

    Serial.print("[CMD] Convertido a celdas: x=");
    Serial.print(destino.x);
    Serial.print(", y=");
    Serial.println(destino.y);

    robot.crearNuevaMision(destino);
}

void procesarComandoG(String comando, Robot &robot) {
    int x = 0;
    int y = 0;

    if (!extraerDosEnteros(comando, x, y)) {
        reportar("Formato invalido. Usa: G x y");
        return;
    }

    Celda destino = {x, y};

    Serial.print("[CMD] Destino en celdas recibido: x=");
    Serial.print(destino.x);
    Serial.print(", y=");
    Serial.println(destino.y);

    robot.crearNuevaMision(destino);
}

void leerComandos(Robot &robot) {
    if (!Serial.available()) {
        return;
    }

    String comando = Serial.readStringUntil('\n');
    comando.trim();

    if (comando.length() == 0) {
        return;
    }

    comando.toUpperCase();

    Serial.print("[CMD] Recibido: ");
    Serial.println(comando);

    if (comando == "AYUDA") {
        mostrarAyuda();
    }

    else if (comando == "ESTADO") {
        robot.imprimirEstado();
    }

    else if (comando.startsWith("GC ")) {
        procesarComandoGC(comando, robot);
    }

    else if (comando.startsWith("G ")) {
        procesarComandoG(comando, robot);
    }

    else if (comando == "S") {
        robot.detener();
    }

    else if (comando == "D") {
        robot.solicitarDescargaManual();
    }

    else if (comando == "RESET") {
        robot.inicializar();
        reportar("Estado logico reiniciado");
    }

    else if (comando == "F") {
        pruebaAvanzar();
    }

    else if (comando == "B") {
        pruebaRetroceder();
    }

    else if (comando == "L") {
        pruebaGirarIzquierda();
    }

    else if (comando == "R") {
        pruebaGirarDerecha();
    }

    else if (comando == "E") {
        imprimirEncoders();
    }

    else if (comando == "ER") {
        resetEncoders();
        reportar("Encoders reseteados");
    }

    else if (comando == "IMU") {
        imprimirIMU();
    }

    else if (comando == "YAW0") {
        resetYaw();
        reportar("Yaw relativo reseteado");
    }

    else if (comando == "U") {
        imprimirUltrasonidos();
    }

    else if (comando == "SERVO") {
        imprimirEstadoServo();
    }

    else if (comando == "ABRIR") {
        abrirCarga();
    }

    else if (comando == "CERRAR") {
        cerrarCarga();
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

    else if (comando == "MAPA") {
        robot.imprimirMapa();
    }

    else if (comando == "SENSAR") {
        robot.actualizarMapaSensoresManual();
    }

    else if (comando == "RUTA") {
        robot.imprimirRutaActual();
    }

    else {
        reportar("Comando no reconocido. Escribe AYUDA.");
    }
}