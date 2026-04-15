# LOGS - Registro de Decisiones de Diseño

## Fecha: 2026-04-15

### Decisiones Iniciales

1. **Arquitectura de tareas**: Se usa FreeRTOS con 3 tasks:
   - `motorTask` (prioridad 2): Control del stepper
   - `serialTask` (prioridad 3): Procesamiento de comandos
   - `wifiTask` (prioridad 1): Reservado para etapa 2

2. **Comunicación**: Protocolo JSON sobre UART a 115200 baudios

3. **Control de motor**: Uso de señales STEP/DIR directamente (no UART del TMC2226 para etapa 1)

4. **Expansor I2C**: PCA9555 en dirección 0x20 para gestionar pines de control del driver

5. **Microstepping por defecto**: 1/16 (16 micropasos) - buen balance entre silencio y precisión

6. **Cálculo de flujo**: Basado en volumen por revolución del mecanismo tornillo-jeringa

### Siguiente Iteración
- Crear estructura de directorios firmware/
- Implementar código inicial PlatformIO
- Implementar cliente Python básico

---

## 2026-04-15 - Iteration 1: Implementación Inicial

### Acciones Tomadas
1. Creada estructura de directorios según AGENTS.md
2. Implementado FSD.md con todas las especificaciones técnicas
3. Creado código firmware PlatformIO modular (main.cpp, MotorController, CommandParser, SerialHandler)
4. Creado cliente Python CLI y tests de validación
5. Tests Python: 17/17 PASARON

### Pendientes para pruebas hardware
- Verificar conexión UART con ESP32
- Validar comunicación I2C con PCA9555
- Probar control de motor stepper via TMC2226
- Ajustar parámetros de flujo según motor real

---

## 2026-04-15 - Iteration 2: Correcciones

### Cambios Realizados
1. **Pines PCA9555 corregidos**: 
   - EN -> P15, MS1 (ADDR1) -> P14, MS2 (ADDR2) -> P13
   - Actualizado en config.h y FSD.md

2. **ArduinoJson integrado**:
   - Agregada dependencia bblanchon/ArduinoJson@^6.21 en platformio.ini
   - CommandParser reescrito para usar deserializeJson

3. **Microstepping vía UART**:
   - Microstepping se configurará vía UART del TMC2226 (TMCStepper library)
   - Pines MS1/MS2 del PCA9555 ahora funcionan como address pins

---

## 2026-04-15 - Iteration 3: Compilación Exitosa

### Cambios Realizados
1. **Integración ContinuousStepper**: Corregido uso del template `ContinuousStepper<StepperDriver, LoopTicker>`
2. **Integración TMCStepper**: Corregido constructor y métodos (rms_current en lugar de hold_current)
3. **API ContinuousStepper actualizada**: Cambiados setSpeed->spin, runForward/runReverse->spin(pos/neg)
4. **Librería clsPCA9555**: Agregados defines por defecto para PCA9555_ADDR, I2C_SDA, I2C_SCL
5. **C++17 habilitado**: Agregado flag `-std=gnu++17` en platformio.ini
6. **CommandParser**: Agregado parseo de parámetro `address`

### Resultados de Compilación
```
RAM:   6.7% (22112 bytes)
Flash: 24.9% (326509 bytes)
Estado: SUCCESS
```

---

## 2026-04-15 - Iteration 4: Pruebas Hardware

### Resultados de Pruebas
| Test | Resultado |
|------|-----------|
| Upload firmware | SUCCESS |
| Status command | OK |
| Config command | OK |
| Start command | OK |
| Stop command | OK |

### Observaciones
- Comunicación UART funcionando a 115200 baud
- JSON parseo OK
- Motor responde a comandos básicos

### Pendientes
- Verificar movimiento físico del motor
- Probar con diferentes valores de microstepping
- Validar precisión de flujo

---

## Histórico de Cambios

| Fecha | Versión | Cambio |
|-------|---------|--------|
| 2026-04-15 | 1.0 | Creación inicial FSD |
| 2026-04-15 | 1.1 | Corrección pines PCA9555 e integración ArduinoJson |
| 2026-04-15 | 1.2 | Compilación exitosa, integración librerías |
| 2026-04-15 | 1.3 | Pruebas hardware exitosas |
