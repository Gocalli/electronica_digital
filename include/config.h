#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- CONFIGURACIÓN DE DEPURACIÓN ---
#define DEBUG_MODE        true   // Cambiar a false para hardware real
#define SIMULAR_AVANCE_TIEMPO 60  // Minutos que "pasan" en cada ciclo de debug

// --- PINES DE PERIFÉRICOS ---
#define PIN_HUMEDAD_SUELO 34   // ADC
#define PIN_RELE_BOMBA    26   // GPIO Salida
#define PIN_SD_CS         5    // SPI Chip Select

// Pines I2C (ESP32 Defaults: SDA=21, SCL=22)
#define I2C_SDA 21
#define I2C_SCL 22

// --- CONSTANTES DEL SISTEMA ---
#define INTERVALO_LECTURA 900000 // 15 minutos en ms (15 * 60 * 1000)
#define MUESTRAS_PROMEDIO 10     // Cantidad de lecturas para el filtro

// --- CONFIGURACIÓN OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

// --- RESTRICCIONES HORARIAS ---
#define HORA_INICIO_PROHIBIDO 18 // 18:00
#define HORA_FIN_PROHIBIDO    6  // 06:00

#endif
