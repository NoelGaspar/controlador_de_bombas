#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Pines del TMC2226 (STEP y DIR)
#define PIN_STEP           4
#define PIN_DIR            23

// Pines del PCA9555 para control de enable y address
#define PCA_ADDR_PIN_EN    15
#define PCA_ADDR_PIN_MS1   14
#define PCA_ADDR_PIN_MS2   13

// Configuración I2C
#define I2C_SDA            21
#define I2C_SCL            22
#define PCA9555_ADDR       0x20

// Lógica del Enable (HIGH = habilitado, LOW = deshabilitado para TMC2226)
#define TMC_ENABLE_ON      LOW
#define TMC_ENABLE_OFF     HIGH

// Configuración UART
#define UART_BAUD          115200

// Configuración del Motor
#define DEFAULT_MICROSTEP  16
#define STEPS_PER_REV       200
#define DEFAULT_DIAMETER   10.0f
#define DEFAULT_REDUCTION  1.0f
#define PITCH_TORNILLO     2.0f

// Tareas FreeRTOS
#define MOTOR_TASK_STACK   4096
#define SERIAL_TASK_STACK   4096
#define MOTOR_TASK_PRIO     2
#define SERIAL_TASK_PRIO    3

#endif