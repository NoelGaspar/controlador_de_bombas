# FSD - Functional Specification Document

## 1. Visión General del Proyecto

**Nombre**: Controlador de Bombas de Jeringa  
**Tipo**: Sistema embebido de control de motores stepper para instrumentación científica  
**Resumen**: Sistema basado en ESP32 que controla uno o más motores stepper NEMA 17 accionando jeringas mediante mecanismo de tornillo sin fin, permitiendo dispensar líquidos con control de flujo volumétrico.  
**Usuario objetivo**: Laboratorios científicos, sistemas de automatización, dispositivos médicos

---

## 2. Arquitectura del Sistema

### 2.1 Hardware

| Componente | Modelo | Especificación |
|------------|--------|----------------|
| MCU | ESP32 | WiFi + Bluetooth, dual-core |
| Driver stepper | TMC2226 | UART config, microstepping 1/256 |
| Motor stepper | NEMA 17 | 1.8°/step, 1.5-2A bipolar |
| IO Expander | PCA9555 | I2C addr 0x20, 16 pines |
| Mecanismo | Tornillo sin fin | Jeringa 10ml estándar |

### 2.2 Asignación de Pines

**Nota**: Los pines MS1 y MS2 del PCA9555 se usan para configurar la dirección I2C del driver. El microstepping se configura vía UART del TMC2226.

```
TMC2226         -> ESP32
----------------|-------
EN              -> PCA9555 P15 (I2C addr 0x20)
MS1             -> PCA9555 P14 (ADDR pin para I2C)
MS2             -> PCA9555 P13 (ADDR pin para I2C)
STEP            -> GPIO4
DIR             -> GPIO23
PDN             -> GPIO16
UART_TX         -> GPIO17
UART_RX         -> GPIO16
CLK             -> NC (no conectado)

PCA9555 -> ESP32 (I2C)
--------|----------
SDA     -> GPIO21
SCL     -> GPIO22
```

### 2.2.1 Configuración de Microstepping

El microstepping se configura vía UART del TMC2226:
- Comando: `"G"` + valor de microstepping
- Valores: 0=full, 1=half, 2=quarter, 3=eighth, 4=sixteenth, 5=thirty-second, 6=sixty-fourth, 8=256

### 2.2.2 Pines lógicos del PCA9555 (usados como address I2C del driver)

| Pin PCA9555 | Función | Descripción |
|-------------|---------|-------------|
| P15 | EN | Enable del driver |
| P14 | ADDR1 | Bit 1 dirección I2C |
| P13 | ADDR2 | Bit 2 dirección I2C |

### 2.3 Diagrama de Bloques

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   Host PC   │────>│    ESP32    │────>│ TMC2226     │
│  (Python)   │ UART│  (Firmware)│ STEP│ Driver      │
└─────────────┘     └──────┬──────┘     └──────┬──────┘
                           │                    │
                    ┌──────▼──────┐      ┌──────▼──────┐
                    │ PCA9555     │      │ NEMA 17     │
                    │ (I2C IO)    │      │ Stepper     │
                    └─────────────┘      └─────────────┘
```

---

## 3. Especificación de Comunicación

### 3.1 Interfaz UART

- **Baudrate**: 115200
- **Databits**: 8
- **Parity**: None
- **Stopbits**: 1

### 3.2 Protocolo JSON

Todos los comandos y respuestas en formato JSON.

#### Comandos del Host al Dispositivo

| Comando | Descripción | Parámetros |
|---------|--------------|------------|
| `config` | Configurar parámetros de operación | `diameter`, `flow_rate`, `reduction`, `microstep`, `direction` |
| `start` | Iniciar dispensado | ninguno |
| `stop` | Detener motor | ninguno |
| `status` | Consultar estado actual | ninguno |
| `calibrate` | Iniciar rutina de calibración | ninguno |
| `reset` | Reiniciar dispositivo | ninguno |

#### Estructura de Comandos

```json
// config
{"cmd": "config", "diameter": 10.0, "flow_rate": 1.0, "reduction": 1, "microstep": 16, "direction": "inject"}

// start
{"cmd": "start"}

// stop
{"cmd": "stop"}

// status
{"cmd": "status"}

// calibrate
{"cmd": "calibrate"}

// reset
{"cmd": "reset"}
```

#### Respuestas del Dispositivo al Host

```json
// Respuesta genérica de éxito
{"status": "ok", "msg": "Motor iniciado", "data": {}

// Respuesta de estado
{"status": "ok", "msg": "Estado actual", "data": {
    "running": true,
    "direction": "inject",
    "flow_rate": 1.0,
    "current_position": 500,
    "steps_total": 10000
}}

// Respuesta de error
{"status": "error", "msg": "Flujo no puede ser 0", "code": "INVALID_PARAM"}
```

### 3.3 Códigos de Error

| Código | Descripción |
|--------|-------------|
| `INVALID_PARAM` | Parámetro inválido |
| `MOTOR_ERROR` | Error en motor/driver |
| `I2C_ERROR` | Error de comunicación I2C |
| `NOT_INITIALIZED` | Sistema no inicializado |
| `ALREADY_RUNNING` | Motor ya en ejecución |

---

## 4. Especificación Funcional

### 4.1 Modos de Operación

1. **Inyección**: Movimiento del émbolo hacia afuera (avance)
2. **Aspiración**: Movimiento del émbolo hacia adentro (retroceso)

### 4.2 Cálculo de Flujo Volumétrico

**Parámetros de entrada**:
- `diameter`: Diámetro interno de jeringa (mm)
- `flow_rate`: Flujo deseado (ml/min)
- `reduction`: Relación de reducción del mecanismo (1:1 por defecto)
- `microstep`: Factor de microstepping (1, 2, 4, 8, 16, 32, 64, 256)

**Cálculos**:

```
Volumen por revolución del motor = π * (diameter/2)² * (pitch_del_tornillo / reduction)
Pasos por revolución = 200 * microstep
Velocidad del motor (RPM) = flow_rate / Volumen por revolución
Frecuencia de step = (200 * microstep * RPM) / 60
```

### 4.3 Tareas FreeRTOS (Etapa 1)

| Task | Prioridad | Descripción |
|------|-----------|-------------|
| `motorTask` | 2 | Control del motor stepper |
| `serialTask` | 3 | Procesamiento de comandos UART |
| `wifiTask` | 1 | Gestor WiFi (reservado para etapa 2) |

### 4.4 Estados del Motor

```
┌─────────┐
│ IDLE    │───┐
└────┬────┘   │
     │ start  │
     ▼        │
┌─────────┐   │
│ RUNNING  │───┤
└────┬────┘   │ stop
     │        │
     ▼        │
┌─────────┐   │
│ STOPPING│───┘
└─────────┘
```

---

## 5. Estructura del Firmware (PlatformIO)

```
firmware/controlador/
├── lib/
│   ├── PCA9555/
│   │   ├── PCA9555.h
│   │   └── PCA9555.cpp
│   └── ContinuousStepper/
│       └── (biblioteca externa)
├── src/
│   ├── main.cpp
│   ├── MotorController.h
│   ├── MotorController.cpp
│   ├── CommandParser.h
│   ├── CommandParser.cpp
│   ├── SerialHandler.h
│   └── SerialHandler.cpp
├── include/
│   └── config.h
└── platformio.ini
```

---

## 6. Estructura del Software (Python)

```
software/
├── client.py          (CLI para enviar comandos)
├── tests.py           (validación y test)
├── requirements.txt   (dependencias)
├── logs/
│   └── test_log.txt  (resultados de tests)
└── venv/              (entorno virtual)
```

### 6.1 Dependencias Python

- pyserial (comunicación UART)
- requests (para WiFi/HTTP en etapa 2)

---

## 7. Fases de Implementación

### Etapa 1: Validación de Hardware

**Objetivo**: Movimiento básico del motor con comandos

- [ ] Configuración de pines y hardware
- [ ] Comunicación I2C con PCA9555
- [ ] Control básico del TMC2226 (STEP/DIR)
- [ ] Parser de comandos JSON
- [ ] Task FreeRTOS para motor
- [ ] Task FreeRTOS para UART
- [ ] CLI Python básico
- [ ] Test de validación hardware

### Etapa 2: Funcionalidad Avanzada (Futuro)

- [ ] Interfaz GUI Python
- [ ] Modo WiFi/HTTP
- [ ] Rutinas de calibración
- [ ] Control de 4 motores simultáneos
- [ ] Precisión mejorada
- [ ] Almacenamiento persistente (Preferences)

---

## 8. Requisitos No Funcionales

| Requisito | Descripción |
|-----------|-------------|
| Latencia | Comando-respuesta < 100ms |
| Precisión flujo | ±5% del valor configurado |
| Temperatura operación | 0-50°C |
| Voltaje alimentación | 12-24V DC (motor), 5V (lógica) |

---

## 9. Referencias

- ESP32 Arduino Framework: https://docs.platformio.org/en/latest/boards/espressif32/esp32dev.html
- TMC2226 Datasheet: (en docs/datasheets/)
- PCA9555 Datasheet: (en docs/datasheets/)
- ContinuousStepper Library: https://github.com/

---

## 10. Glosario

| Término | Definición |
|---------|------------|
| Microstepping | División de paso completo en micropasos |
| Pitch | Distancia por revolución del tornillo |
| Inyección | Expulsión de líquido (avance émbolo) |
| Aspiración | Succión de líquido (retroceso émbolo) |
| Flow rate | Caudal volumétrico (ml/min) |

