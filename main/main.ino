#include "config.h"
#include "tipos.h"
#include "robot.h"
#include "comandos.h"

Robot robot;

void setup() {
    iniciarComunicacion();

    reportar("Iniciando robot...");
    robot.inicializar();
    reportar("Robot listo");
}

void loop() {
    leerComandos(robot);
    robot.actualizar();
}