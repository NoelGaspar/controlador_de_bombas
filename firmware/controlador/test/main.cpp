#include <Arduino.h>
#include "config.h"
#include "MotorController.h"
#include "SerialHandler.h"

MotorController motor;
SerialHandler serialHandler(motor);

void setup() {
    Serial.begin(UART_BAUD);
    delay(500);
    
    Serial.println("{\"status\":\"ok\",\"msg\":\"Iniciando sistema...\"}");
    
    if (!motor.begin()) {
        Serial.println("{\"status\":\"error\",\"msg\":\"Error inicializando\",\"code\":\"INIT_ERROR\"}");
        while (true) {
            delay(1000);
        }
    }
    
    Serial.println("{\"status\":\"ok\",\"msg\":\"Sistema iniciado\",\"data\":{\"version\":\"1.0\"}}");
}

void loop() {
    serialHandler.loop();
    motor.loop();
}