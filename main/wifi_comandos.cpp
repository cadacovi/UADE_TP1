#include "wifi_Comandos.h"

#include <WiFi.h>
#include <WebServer.h>

#include "config.h"
#include "tipos.h"
#include "control_movimiento.h"
#include "encoders.h"
#include "imu.h"
#include "ultrasonidos.h"
#include "motores.h"
#include "servo_carga.h"

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
        detenerMotores();

        if (robotWiFi != nullptr) {
            robotWiFi->detener();
        }

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

    // ---------- Control manual ----------

    else if (comando == "MAN_AV") {
        ResultadoAvance r = avanzarDistanciaResultado(WIFI_MANUAL_AVANCE_MM);

        if (r.exito) {
            return "Manual: avance completado";
        }

        if (r.obstaculoDetectado) {
            return "Manual: avance detenido por obstaculo";
        }

        return "Manual: avance fallido";
    }

    else if (comando == "MAN_RET") {
        ResultadoAvance r = avanzarDistanciaResultado(-WIFI_MANUAL_AVANCE_MM);

        if (r.exito) {
            return "Manual: retroceso completado";
        }

        return "Manual: retroceso fallido";
    }

    else if (comando == "MAN_DER") {
        ResultadoGiro r = girarAnguloResultado(WIFI_MANUAL_GIRO_GRADOS);

        if (r.exito) {
            return "Manual: giro derecha completado";
        }

        return "Manual: giro derecha fallido";
    }

    else if (comando == "MAN_IZQ") {
        ResultadoGiro r = girarAnguloResultado(-WIFI_MANUAL_GIRO_GRADOS);

        if (r.exito) {
            return "Manual: giro izquierda completado";
        }

        return "Manual: giro izquierda fallido";
    }

    else if (comando == "MAN_DER45") {
        ResultadoGiro r = girarAnguloResultado(WIFI_MANUAL_GIRO_MEDIO_GRADOS);
        return r.exito ? "Manual: giro derecha 45 completado" : "Manual: giro derecha 45 fallido";
    }

    else if (comando == "MAN_IZQ45") {
        ResultadoGiro r = girarAnguloResultado(-WIFI_MANUAL_GIRO_MEDIO_GRADOS);
        return r.exito ? "Manual: giro izquierda 45 completado" : "Manual: giro izquierda 45 fallido";
    }

    else if (comando == "MAN_DER10") {
        ResultadoGiro r = girarAnguloResultado(WIFI_MANUAL_GIRO_FINO_GRADOS);
        return r.exito ? "Manual: giro derecha 10 completado" : "Manual: giro derecha 10 fallido";
    }

    else if (comando == "MAN_IZQ10") {
        ResultadoGiro r = girarAnguloResultado(-WIFI_MANUAL_GIRO_FINO_GRADOS);
        return r.exito ? "Manual: giro izquierda 10 completado" : "Manual: giro izquierda 10 fallido";
    }

    return "Comando no reconocido: " + comando;
}

static String paginaHTML() {
    String html = "";

    html += "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>Robot ESP32-S3</title>";

    html += "<style>";
    html += "body{font-family:Arial,Helvetica,sans-serif;background:#0f172a;color:#e5e7eb;margin:0;padding:18px;}";
    html += "h1{font-size:26px;margin:0 0 6px 0;}";
    html += "h2{font-size:18px;margin-top:22px;border-bottom:1px solid #334155;padding-bottom:6px;}";
    html += ".sub{color:#94a3b8;margin-bottom:18px;}";
    html += ".card{background:#111827;border:1px solid #334155;border-radius:14px;padding:14px;margin-bottom:14px;box-shadow:0 4px 14px rgba(0,0,0,.25);}";
    html += ".grid{display:grid;grid-template-columns:1fr 1fr;gap:10px;}";
    html += ".grid3{display:grid;grid-template-columns:1fr 1fr 1fr;gap:10px;align-items:center;}";
    html += "button{font-size:17px;padding:13px 10px;border:0;border-radius:12px;background:#2563eb;color:white;font-weight:bold;}";
    html += "button:active{transform:scale(.97);}";
    html += ".stop{background:#dc2626;color:white;font-size:20px;}";
    html += ".ok{background:#16a34a;}";
    html += ".warn{background:#ca8a04;}";
    html += ".gray{background:#475569;}";
    html += ".purple{background:#7c3aed;}";
    html += "input{font-size:17px;padding:12px;border-radius:10px;border:1px solid #475569;background:#020617;color:#e5e7eb;width:calc(100% - 24px);margin-bottom:10px;}";
    html += "pre{background:#020617;border:1px solid #334155;color:#d1d5db;padding:12px;border-radius:12px;white-space:pre-wrap;min-height:90px;}";
    html += ".small button{font-size:14px;padding:10px;}";
    html += "</style>";

    html += "</head><body>";

    html += "<h1>Robot autonomo ESP32-S3</h1>";
    html += "<div class='sub'>Panel de control para demostracion</div>";

    html += "<div class='card'>";
    html += "<button class='stop' style='width:100%;' onclick=\"cmd('S')\">STOP</button>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<h2>Control manual</h2>";

    html += "<div class='grid3'>";
    html += "<div></div>";
    html += "<button class='ok' onclick=\"cmd('MAN_AV')\">Avanzar</button>";
    html += "<div></div>";

    html += "<button class='warn' onclick=\"cmd('MAN_IZQ')\">Izq 90</button>";
    html += "<button class='stop' onclick=\"cmd('S')\">Stop</button>";
    html += "<button class='warn' onclick=\"cmd('MAN_DER')\">Der 90</button>";

    html += "<div></div>";
    html += "<button class='ok' onclick=\"cmd('MAN_RET')\">Retroceder</button>";
    html += "<div></div>";
    html += "</div>";

    html += "<h2>Correccion de giro</h2>";
    html += "<div class='grid'>";
    html += "<button class='warn' onclick=\"cmd('MAN_IZQ45')\">Izq 45</button>";
    html += "<button class='warn' onclick=\"cmd('MAN_DER45')\">Der 45</button>";
    html += "<button class='gray' onclick=\"cmd('MAN_IZQ10')\">Izq 10</button>";
    html += "<button class='gray' onclick=\"cmd('MAN_DER10')\">Der 10</button>";
    html += "</div>";

    html += "</div>";

    html += "<div class='card'>";
    html += "<h2>Misiones rapidas</h2>";
    html += "<div class='grid'>";
    html += "<button onclick=\"cmd('G 0 1')\">G 0 1</button>";
    html += "<button onclick=\"cmd('G 0 2')\">G 0 2</button>";
    html += "<button onclick=\"cmd('G 1 0')\">G 1 0</button>";
    html += "<button onclick=\"cmd('G 1 1')\">G 1 1</button>";
    html += "</div>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<h2>Enviar comando</h2>";
    html += "<input id='entrada' placeholder='Ej: G 1 1, ESTADO, U'>";
    html += "<button style='width:100%;' onclick=\"cmdInput()\">Enviar</button>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<h2>Acciones</h2>";
    html += "<div class='grid'>";
    html += "<button class='purple' onclick=\"cmd('ESTADO')\">Estado</button>";
    html += "<button class='purple' onclick=\"cmd('U')\">Sensores US</button>";
    html += "<button class='gray' onclick=\"cmd('D')\">Descargar</button>";
    html += "<button class='gray' onclick=\"cmd('AYUDA')\">Ayuda</button>";
    html += "</div>";
    html += "</div>";

    html += "<div class='card small'>";
    html += "<h2>Debug</h2>";
    html += "<div class='grid'>";
    html += "<button onclick=\"cmd('YAW0')\">Reset Yaw</button>";
    html += "<button onclick=\"cmd('ER')\">Reset Encoders</button>";
    html += "<button onclick=\"cmd('AVCELDA')\">Av. celda</button>";
    html += "<button onclick=\"cmd('G180')\">Giro 180</button>";
    html += "</div>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<h2>Respuesta</h2>";
    html += "<pre id='respuesta'>";
    html += ultimaRespuesta;
    html += "</pre>";
    html += "</div>";

    html += "<script>";
    html += "function cmd(c){";
    html += "document.getElementById('respuesta').textContent='Ejecutando: '+c+'...';";
    html += "fetch('/cmd?c='+encodeURIComponent(c))";
    html += ".then(r=>r.text())";
    html += ".then(t=>{document.getElementById('respuesta').textContent=t;})";
    html += ".catch(e=>{document.getElementById('respuesta').textContent='Error de conexion';});";
    html += "}";
    html += "function cmdInput(){";
    html += "let v=document.getElementById('entrada').value;";
    html += "if(v.trim().length>0) cmd(v);";
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