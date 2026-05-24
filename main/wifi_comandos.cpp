#include "wifi_Comandos.h"

#include <WiFi.h>
#include <WebServer.h>

#include "config.h"
#include "tipos.h"
#include "control_movimiento.h"
#include "encoders.h"
#include "imu.h"
#include "ultrasonidos.h"

static WebServer servidor(80);
static Robot* robotWiFi = nullptr;

static String ultimaRespuesta = "Sistema iniciado";

static const char* estadoComoTextoWiFi(EstadoRobot estado) {
    switch (estado) {
        case IDLE: return "IDLE";
        case PLANIFICANDO_RUTA: return "PLANIFICANDO_RUTA";
        case NAVEGANDO: return "NAVEGANDO";
        case EVITANDO_OBSTACULO: return "EVITANDO_OBSTACULO";
        case DESCARGANDO: return "DESCARGANDO";
        case MISION_COMPLETADA: return "MISION_COMPLETADA";
        case ESTADO_ERROR: return "ERROR";
        default: return "DESCONOCIDO";
    }
}

static const char* direccionComoTextoWiFi(Direccion direccion) {
    switch (direccion) {
        case NORTE: return "NORTE";
        case ESTE: return "ESTE";
        case SUR: return "SUR";
        case OESTE: return "OESTE";
        default: return "DESCONOCIDA";
    }
}

static bool extraerDosEnteros(String comando, int &a, int &b) {
    comando.trim();

    int espacio1 = comando.indexOf(' ');
    if (espacio1 < 0) return false;

    int espacio2 = comando.indexOf(' ', espacio1 + 1);
    if (espacio2 < 0) return false;

    String parteA = comando.substring(espacio1 + 1, espacio2);
    String parteB = comando.substring(espacio2 + 1);

    parteA.trim();
    parteB.trim();

    if (parteA.length() == 0 || parteB.length() == 0) return false;

    a = parteA.toInt();
    b = parteB.toInt();

    return true;
}

static String generarEstadoRobot() {
    if (robotWiFi == nullptr) {
        return "Robot no inicializado";
    }

    Celda pos = robotWiFi->obtenerPosicionActual();
    Celda dst = robotWiFi->obtenerDestino();

    String texto = "";

    texto += "Estado: ";
    texto += estadoComoTextoWiFi(robotWiFi->obtenerEstadoActual());
    texto += "\n";

    texto += "Posicion: x=";
    texto += pos.x;
    texto += " y=";
    texto += pos.y;
    texto += "\n";

    texto += "Destino: x=";
    texto += dst.x;
    texto += " y=";
    texto += dst.y;
    texto += "\n";

    texto += "Direccion: ";
    texto += direccionComoTextoWiFi(robotWiFi->obtenerDireccionActual());
    texto += "\n";

    texto += "Yaw: ";
    texto += String(obtenerYawRelativo(), 2);
    texto += " deg\n";

    texto += "Pulsos I/D: ";
    texto += obtenerPulsosIzquierdo();
    texto += " / ";
    texto += obtenerPulsosDerecho();
    texto += "\n";

    texto += "Distancia mm I/D: ";
    texto += String(obtenerDistanciaIzquierdaMm(), 1);
    texto += " / ";
    texto += String(obtenerDistanciaDerechaMm(), 1);
    texto += "\n";

    return texto;
}

static String ejecutarComandoWiFi(String comando) {
    if (robotWiFi == nullptr) {
        return "Error: robot no inicializado";
    }

    comando.trim();
    comando.toUpperCase();

    if (comando.length() == 0) {
        return "Comando vacio";
    }

    Serial.print("[WIFI CMD] ");
    Serial.println(comando);

    if (comando == "AYUDA") {
        return
            "Comandos:\n"
            "ESTADO\n"
            "G x y\n"
            "GC x_cm y_cm\n"
            "S\n"
            "D\n"
            "AV100\n"
            "AV200\n"
            "AVCELDA\n"
            "GDER\n"
            "GIZQ\n"
            "G180\n"
            "ER\n"
            "YAW0\n"
            "U\n";
    }

    else if (comando == "ESTADO") {
        return generarEstadoRobot();
    }

    else if (comando.startsWith("GC ")) {
        int xCm = 0;
        int yCm = 0;

        if (!extraerDosEnteros(comando, xCm, yCm)) {
            return "Formato invalido. Usa: GC x_cm y_cm";
        }

        Celda destino;
        destino.x = (xCm * 10) / TAM_CELDA_MM;
        destino.y = (yCm * 10) / TAM_CELDA_MM;

        robotWiFi->crearNuevaMision(destino);

        String r = "Nueva mision en cm\n";
        r += "x_cm=" + String(xCm) + " y_cm=" + String(yCm) + "\n";
        r += "Destino celdas: x=" + String(destino.x) + " y=" + String(destino.y);

        return r;
    }

    else if (comando.startsWith("G ")) {
        int x = 0;
        int y = 0;

        if (!extraerDosEnteros(comando, x, y)) {
            return "Formato invalido. Usa: G x y";
        }

        Celda destino = {x, y};
        robotWiFi->crearNuevaMision(destino);

        String r = "Nueva mision\n";
        r += "Destino: x=" + String(x) + " y=" + String(y);

        return r;
    }

    else if (comando == "S") {
        robotWiFi->detener();
        return "Robot detenido";
    }

    else if (comando == "D") {
        robotWiFi->solicitarDescargaManual();
        return "Descarga manual solicitada";
    }

    else if (comando == "AV100") {
        bool ok = avanzarDistanciaMm(100.0);
        return ok ? "AV100 completado" : "AV100 fallo";
    }

    else if (comando == "AV200") {
        bool ok = avanzarDistanciaMm(200.0);
        return ok ? "AV200 completado" : "AV200 fallo";
    }

    else if (comando == "AVCELDA") {
        bool ok = avanzarUnaCelda();
        return ok ? "AVCELDA completado" : "AVCELDA fallo";
    }

    else if (comando == "GDER") {
        bool ok = girar90Derecha();
        return ok ? "Giro derecha completado" : "Giro derecha fallo";
    }

    else if (comando == "GIZQ") {
        bool ok = girar90Izquierda();
        return ok ? "Giro izquierda completado" : "Giro izquierda fallo";
    }

    else if (comando == "G180") {
        bool ok = girar180();
        return ok ? "Giro 180 completado" : "Giro 180 fallo";
    }

    else if (comando == "ER") {
        resetEncoders();
        return "Encoders reseteados";
    }

    else if (comando == "YAW0") {
        resetYaw();
        return "Yaw reseteado";
    }

    else if (comando == "U") {
        String r = "";
        r += "Frontal mm: ";
        r += String(leerFrontalFiltradoRapidoMm(), 1);
        r += "\nIzquierdo mm: ";
        r += String(leerIzquierdoFiltradoMm(), 1);
        r += "\nDerecho mm: ";
        r += String(leerDerechoFiltradoMm(), 1);
        return r;
    }

    return "Comando no reconocido: " + comando;
}

static String paginaHTML() {
    String html = "";

    html += "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>Robot ESP32</title>";

    html += "<style>";
    html += "body{font-family:Arial;background:#111;color:#eee;margin:20px;}";
    html += "h1{font-size:24px;}";
    html += "button{font-size:18px;margin:5px;padding:12px 16px;border-radius:8px;border:0;}";
    html += "input{font-size:18px;padding:10px;width:80%;}";
    html += "pre{background:#222;padding:12px;border-radius:8px;white-space:pre-wrap;}";
    html += ".grid{display:grid;grid-template-columns:1fr 1fr;gap:6px;max-width:500px;}";
    html += ".danger{background:#d33;color:white;}";
    html += ".ok{background:#2d7;color:#111;}";
    html += "</style>";

    html += "</head><body>";

    html += "<h1>Robot ESP32-S3</h1>";

    html += "<form action='/cmd' method='GET'>";
    html += "<input name='c' placeholder='Ej: G 0 1'>";
    html += "<button type='submit'>Enviar</button>";
    html += "</form>";

    html += "<h2>Comandos rapidos</h2>";
    html += "<div class='grid'>";

    html += "<button onclick=\"cmd('ESTADO')\">ESTADO</button>";
    html += "<button onclick=\"cmd('S')\" class='danger'>STOP</button>";

    html += "<button onclick=\"cmd('G 0 1')\">G 0 1</button>";
    html += "<button onclick=\"cmd('G 1 0')\">G 1 0</button>";

    html += "<button onclick=\"cmd('AV100')\">AV100</button>";
    html += "<button onclick=\"cmd('AV200')\">AV200</button>";

    html += "<button onclick=\"cmd('GIZQ')\">GIZQ</button>";
    html += "<button onclick=\"cmd('GDER')\">GDER</button>";

    html += "<button onclick=\"cmd('ER')\">ER</button>";
    html += "<button onclick=\"cmd('YAW0')\">YAW0</button>";

    html += "<button onclick=\"cmd('U')\">US</button>";
    html += "<button onclick=\"cmd('D')\">DESCARGA</button>";

    html += "</div>";

    html += "<h2>Respuesta</h2>";
    html += "<pre id='respuesta'>";
    html += ultimaRespuesta;
    html += "</pre>";

    html += "<script>";
    html += "function cmd(c){";
    html += "fetch('/cmd?c='+encodeURIComponent(c))";
    html += ".then(r=>r.text())";
    html += ".then(t=>{document.getElementById('respuesta').textContent=t;});";
    html += "}";
    html += "</script>";

    html += "</body></html>";

    return html;
}

static void manejarRaiz() {
    servidor.send(200, "text/html", paginaHTML());
}

static void manejarComando() {
    if (!servidor.hasArg("c")) {
        servidor.send(400, "text/plain", "Falta parametro c");
        return;
    }

    String comando = servidor.arg("c");
    ultimaRespuesta = ejecutarComandoWiFi(comando);

    servidor.send(200, "text/plain", ultimaRespuesta);
}

void iniciarWiFiComandos(Robot &robot) {
    robotWiFi = &robot;

    WiFi.mode(WIFI_AP);

    bool ok = WiFi.softAP(
        WIFI_AP_SSID,
        WIFI_AP_PASSWORD,
        WIFI_AP_CANAL,
        WIFI_AP_OCULTO,
        WIFI_AP_MAX_CLIENTES
    );

    if (!ok) {
        Serial.println("[WIFI] Error iniciando Access Point");
        return;
    }

    IPAddress ip = WiFi.softAPIP();

    Serial.println();
    Serial.println("===== WIFI ROBOT =====");
    Serial.print("SSID: ");
    Serial.println(WIFI_AP_SSID);
    Serial.print("Password: ");
    Serial.println(WIFI_AP_PASSWORD);
    Serial.print("IP: ");
    Serial.println(ip);
    Serial.println("======================");
    Serial.println();

    servidor.on("/", manejarRaiz);
    servidor.on("/cmd", manejarComando);

    servidor.begin();

    Serial.println("[WIFI] Servidor iniciado");
}

void actualizarWiFiComandos() {
    servidor.handleClient();
}