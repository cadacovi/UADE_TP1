#ifndef CONFIG_H
#define CONFIG_H

// =====================================================
// COMUNICACION
// =====================================================

#define SERIAL_BAUDRATE 115200

// =====================================================
// PINOUT ESP32-S3
// =====================================================

// -------------------------
// I2C - MPU6050 / GY-521
// -------------------------

#define PIN_SDA 8
#define PIN_SCL 9

// -------------------------
// Ultrasonicos HC-SR04
// ECHO pasa por 74HC4050
// -------------------------

#define TRIG_FRONT 4
#define ECHO_FRONT 5

#define TRIG_LEFT 6
#define ECHO_LEFT 7

#define TRIG_RIGHT 15
#define ECHO_RIGHT 16

// -------------------------
// Encoders FC-03
// -------------------------

#define ENCODER_LEFT 17
#define ENCODER_RIGHT 18

// -------------------------
// Driver DRV8833
// -------------------------

#define PIN_NSLEEP 10

#define AIN1 11
#define AIN2 12

#define BIN1 13
#define BIN2 14

// -------------------------
// Servo MG90S
// -------------------------

#define SERVO_PIN 21

// =====================================================
// MAPA LOCAL DE MISION
// =====================================================

// Cada celda representa 20 cm
#define TAM_CELDA_MM 250

// Mapa local temporal.
// Se usa solo durante una misión.
#define MAPA_FILAS 11
#define MAPA_COLUMNAS 11

// Origen lógico dentro de la matriz.
// Como el mapa es 11x11, el centro es 5,5.
#define MAPA_ORIGEN_X 5
#define MAPA_ORIGEN_Y 5

// =====================================================
// SISTEMA DE COORDENADAS LOCAL
// =====================================================

// Convención usada en el proyecto:
//
// x positivo = derecha respecto al inicio de la misión
// x negativo = izquierda respecto al inicio de la misión
// y positivo = adelante respecto al inicio de la misión
// y negativo = atrás respecto al inicio de la misión
//
// El robot inicia cada misión en:
// posicion = (0, 0)
// direccion = NORTE local

// =====================================================
// UMBRALES DE OBSTACULOS
// =====================================================

#define DIST_OBSTACULO_FRENTE_MM 180
#define DIST_OBSTACULO_LATERAL_MM 150

// Distancia máxima considerada válida para los HC-SR04.
// Lecturas mayores pueden tratarse como "sin obstáculo cercano".
#define DIST_ULTRASONICO_MAX_MM 2000

// Cantidad de muestras para filtrado.
// Mejor usar número impar: 3, 5, 7...
#define ULTRASONICO_MUESTRAS 5

// =====================================================
// CONSTANTES FISICAS DEL ROBOT
// Se deben calibrar en pruebas reales
// =====================================================

// Diámetro estimado de las ruedas
#define DIAMETRO_RUEDA_MM 65.0

// Pulsos por vuelta del encoder.
// Este valor se debe medir realmente.
#define PULSOS_POR_VUELTA 20.0

// Distancia entre las ruedas izquierda y derecha.
// Sirve para estimar giros y control diferencial.
#define DISTANCIA_ENTRE_RUEDAS_MM 120.0

// Valor aproximado de PI
#define PI_ROBOT 3.14159265

// Perímetro de la rueda
// #define PERIMETRO_RUEDA_MM (PI_ROBOT * DIAMETRO_RUEDA_MM)

// Conversión aproximada de pulsos a distancia.
// Puede recalibrarse luego.
#define PULSOS_POR_MM 0.187   // Calibrado 22/05/26

// Pulsos necesarios para avanzar una celda
#define PULSOS_POR_CELDA (TAM_CELDA_MM * PULSOS_POR_MM)

// =====================================================
// PWM Y CONTROL DE MOTORES
// =====================================================

/* 
// PWM mínimo útil.
// Debe ser suficiente para vencer la fricción.
#define PWM_MIN 80

// PWM máximo permitido.
#define PWM_MAX 255

// Velocidades base iniciales.
// Luego se calibran.
#define PWM_BASE_AVANCE 150
#define PWM_BASE_GIRO 140

// Límites específicos para control de avance
#define PWM_AVANCE_MIN 90
#define PWM_AVANCE_MAX 190

// Límites específicos para giro
#define PWM_GIRO_MIN 85
#define PWM_GIRO_MAX 180 
*/

// =====================================================
// PERFIL DE BATERIA / PWM
// =====================================================

#define PERFIL_BATERIA_BAJA true

#if PERFIL_BATERIA_BAJA

    #define PWM_MIN 160
    #define PWM_MAX 255

    #define PWM_BASE_AVANCE 200
    #define PWM_BASE_GIRO 190

    #define PWM_FINAL_AVANCE 120
    #define PWM_FINAL_GIRO 120

    #define PWM_AVANCE_MIN 130
    #define PWM_AVANCE_MAX 255

    #define PWM_GIRO_MIN 140
    #define PWM_GIRO_MAX 255

    #else

    #define PWM_MIN 120
    #define PWM_MAX 255

    #define PWM_BASE_AVANCE 160
    #define PWM_BASE_GIRO 150

    #define PWM_FINAL_AVANCE 100
    #define PWM_FINAL_GIRO 100

    #define PWM_AVANCE_MIN 120
    #define PWM_AVANCE_MAX 220

    #define PWM_GIRO_MIN 120
    #define PWM_GIRO_MAX 220

#endif

// =====================================================
// CONTROL DE AVANCE
// =====================================================

// Tolerancia aceptada al avanzar una distancia
#define TOLERANCIA_DISTANCIA_MM 20.0

// Distancia hacia el objetivo de avance para bajar pwm
#define DISTANCIA_FRENADO_MM 70.0

// Ganancia para corregir diferencia entre ruedas
#define KP_ENCODER_AVANCE 1.5

// Ganancia para corregir desviación angular usando IMU
#define KP_YAW_AVANCE 1.5

// Tiempo máximo para avanzar una celda.
// Evita que el robot quede intentando avanzar para siempre.
#define TIMEOUT_AVANCE_CELDA_MS 4000

// =====================================================
// CONTROL DE GIRO
// =====================================================

// Tolerancia angular aceptable
#define TOLERANCIA_GIRO_GRADOS 1.0

// Tolerancia angular para corrección fina después de un giro
#define TOLERANCIA_GIRO_FINO_GRADOS 1.0

// Ganancia proporcional para giro
#define KP_GIRO 2.0

// Tiempo máximo para un giro de 90 o 180 grados
#define TIMEOUT_GIRO_MS 4000

// Tolerancia angular para el avance
#define TOLERANCIA_CORRECCION_YAW 1.0
#define MAX_CORRECCION_YAW 90.0

// =====================================================
// MPU6050 / IMU
// =====================================================

// Dirección I2C típica del MPU6050
#define MPU6050_ADDR 0x68

// Tiempo de calibración inicial del giroscopio
#define TIEMPO_CALIBRACION_IMU_MS 2000

// Cantidad de muestras para calibración
#define MUESTRAS_CALIBRACION_IMU 500

// =====================================================
// SERVO DE DESCARGA
// =====================================================

#define SERVO_ANGULO_CERRADO 0
#define SERVO_ANGULO_ABIERTO 90

#define TIEMPO_APERTURA_SERVO_MS 1200

// Valores típicos para servo en ESP32Servo
#define SERVO_MIN_US 500
#define SERVO_MAX_US 2400
#define SERVO_FRECUENCIA_HZ 50

// =====================================================
// WIFI / ACCESS POINT
// =====================================================

#define WIFI_AP_SSID "RobotESP32"
#define WIFI_AP_PASSWORD "12345678"

#define WIFI_AP_CANAL 1
#define WIFI_AP_OCULTO false
#define WIFI_AP_MAX_CLIENTES 2

// =====================================================
// DEPURACION
// =====================================================

#define DEBUG_GENERAL true
#define DEBUG_COMANDOS true
#define DEBUG_ESTADOS true
#define DEBUG_SENSORES false
#define DEBUG_CONTROL false
#define DEBUG_MAPA false

#endif