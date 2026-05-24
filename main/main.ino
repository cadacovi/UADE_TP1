#include "config.h"
#include "tipos.h"
#include "robot.h"
#include "comandos.h"
#include "wifi_comandos.h"

Robot robot;

void setup() {
    iniciarComunicacion();

    reportar("Iniciando robot...");
    robot.inicializar();

    iniciarWiFiComandos(robot);

    reportar("Robot listo");
}

void loop() {
    leerComandos(robot);        // Serial USB
    actualizarWiFiComandos();   // WiFi
    
    robot.actualizar();
}