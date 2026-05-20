#include "imu.h"

static float offsetGyroZ = 0.0;
static float gyroZ = 0.0;
static float yawRelativo = 0.0;

static unsigned long tiempoAnteriorIMU = 0;
static bool imuInicializada = false;

static int16_t leerRegistro16(uint8_t registroAlto) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(registroAlto);
    Wire.endTransmission(false);

    Wire.requestFrom(MPU6050_ADDR, 2, true);

    int16_t valor = Wire.read() << 8 | Wire.read();
    return valor;
}

bool inicializarIMU() {
    Wire.begin(PIN_SDA, PIN_SCL);

    Wire.beginTransmission(MPU6050_ADDR);
    byte error = Wire.endTransmission();

    if (error != 0) {
        Serial.println("[IMU] MPU6050 no detectado");
        imuInicializada = false;
        return false;
    }

    // Despertar MPU6050
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x6B);   // PWR_MGMT_1
    Wire.write(0x00);   // salir de sleep
    Wire.endTransmission(true);

    delay(100);

    // Configurar gyro ±250 grados/s
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x1B);   // GYRO_CONFIG
    Wire.write(0x00);
    Wire.endTransmission(true);

    imuInicializada = true;
    tiempoAnteriorIMU = micros();

    Serial.println("[IMU] MPU6050 inicializado");

    calibrarIMU();
    resetYaw();

    return true;
}

void calibrarIMU() {
    if (!imuInicializada) {
        Serial.println("[IMU] No se puede calibrar: IMU no inicializada");
        return;
    }

    Serial.println("[IMU] Calibrando giroscopio Z...");
    Serial.println("[IMU] Mantener el robot quieto");

    long suma = 0;

    for (int i = 0; i < MUESTRAS_CALIBRACION_IMU; i++) {
        int16_t rawGyroZ = leerRegistro16(0x47); // GYRO_ZOUT_H
        suma += rawGyroZ;
        delay(3);
    }

    float promedioRaw = suma / (float)MUESTRAS_CALIBRACION_IMU;

    // Sensibilidad para ±250 grados/s = 131 LSB/(°/s)
    offsetGyroZ = promedioRaw / 131.0;

    Serial.print("[IMU] Offset Gyro Z: ");
    Serial.println(offsetGyroZ);
}

void actualizarIMU() {
    if (!imuInicializada) {
        return;
    }

    unsigned long tiempoActual = micros();
    float dt = (tiempoActual - tiempoAnteriorIMU) / 1000000.0;
    tiempoAnteriorIMU = tiempoActual;

    int16_t rawGyroZ = leerRegistro16(0x47);

    gyroZ = (rawGyroZ / 131.0) - offsetGyroZ;

    // Integracion simple del yaw relativo
    yawRelativo += gyroZ * dt;
}

void resetYaw() {
    yawRelativo = 0.0;
    tiempoAnteriorIMU = micros();
}

float obtenerYawRelativo() {
    return yawRelativo;
}

float obtenerGyroZ() {
    return gyroZ;
}

void imprimirIMU() {
    actualizarIMU();

    Serial.println();
    Serial.println("===== IMU MPU6050 =====");

    Serial.print("Gyro Z grados/s: ");
    Serial.println(obtenerGyroZ());

    Serial.print("Yaw relativo grados: ");
    Serial.println(obtenerYawRelativo());

    Serial.print("Offset Gyro Z: ");
    Serial.println(offsetGyroZ);

    Serial.println("=======================");
    Serial.println();
}