#ifndef CONFIG_H
#define CONFIG_H

#define SERIAL_BAUDRATE 115200

// Direcciones comunes de LCD I2C:
// 0x27 suele ser la más común.
// Si no funciona, probar 0x3F.
#define LCD_I2C_ADDR 0x27

#define LCD_COLUMNAS 16
#define LCD_FILAS 2

#endif