CONSTANTES:
    TAM_CELDA_CM = 20
    DIST_OBSTACULO_FRENTE = 18
    DIST_OBSTACULO_LATERAL = 15
    TIEMPO_APERTURA_SERVO_MS = 1200
    MAX_FILAS = N
    MAX_COLUMNAS = M

ENUM EstadoRobot:
    IDLE
    PLANIFICANDO_RUTA
    NAVEGANDO
    EVITANDO_OBSTACULO
    DESCARGANDO
    MISION_COMPLETADA
    ERROR

ENUM Direccion:
    NORTE
    ESTE
    SUR
    OESTE

ENUM EstadoCelda:
    DESCONOCIDA
    LIBRE
    OCUPADA
    VISITADA

TIPO Celda:
    x
    y

TIPO Robot:
    posicion_actual : Celda
    direccion_actual : Direccion
    estado_actual : EstadoRobot
    destino : Celda
    mapa[MAX_FILAS][MAX_COLUMNAS] : EstadoCelda
    ruta_actual : lista de Celda
    carga_entregada : booleano
    comando_descarga_manual : booleano