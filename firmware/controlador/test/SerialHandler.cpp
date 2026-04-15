#include "SerialHandler.h"

SerialHandler::SerialHandler(MotorController& motorRef) 
    : motor(motorRef) {
}

void SerialHandler::begin() {
    Serial.begin(UART_BAUD);
    inputBuffer = "";
    Serial.println("{\"status\":\"ok\",\"msg\":\"Sistema iniciado\"}");
}

void SerialHandler::loop() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (inputBuffer.length() > 0) {
                Command cmd;
                if (parser.parse(inputBuffer, cmd)) {
                    processCommand(cmd);
                } else {
                    sendError(parser.getLastError(), "INVALID_CMD");
                }
                inputBuffer = "";
            }
        } else {
            inputBuffer += c;
        }
    }
}

void SerialHandler::processCommand(const Command& cmd) {
    switch (cmd.type) {
        case CommandType::CONFIG:
            if (motor.configure(cmd.motorConfig)) {
                MotorConfig cfg = motor.getConfig();
                String json = "{\"status\":\"ok\",\"msg\":\"Configuracion aplicada\",\"data\":{";
                json += "\"diameter\":" + String(cfg.diameter_mm);
                json += ",\"flow_rate\":" + String(cfg.flow_rate_ml_min);
                json += ",\"microstep\":" + String(cfg.microstep);
                json += ",\"direction\":\"" + String(cfg.direction == MotorDirection::INJECT ? "inject" : "aspire") + "\"";
                json += "}}";
                sendResponse(json);
            } else {
                sendError("Parametros invalidos", "INVALID_PARAM");
            }
            break;
            
        case CommandType::START:
            if (motor.start()) {
                MotorConfig cfg = motor.getConfig();
                String json = "{\"status\":\"ok\",\"msg\":\"Motor iniciado\",\"data\":{";
                json += "\"flow_rate\":" + String(cfg.flow_rate_ml_min);
                json += ",\"microstep\":" + String(cfg.microstep);
                json += ",\"spinning\":true";
                json += "}}";
                sendResponse(json);
            } else {
                sendError("Motor ya en ejecucion", "ALREADY_RUNNING");
            }
            break;
            
        case CommandType::STOP:
            if (motor.stop()) {
                sendResponse("{\"status\":\"ok\",\"msg\":\"Motor detenido\",\"data\":{\"stepper_stopped\":true}}");
            } else {
                sendError("Motor no esta en ejecucion", "NOT_RUNNING");
            }
            break;
            
        case CommandType::STATUS:
            sendStatus();
            break;
            
        case CommandType::RESET:
            motor.stop();
            sendResponse("{\"status\":\"ok\",\"msg\":\"Sistema reiniciado\"}");
            break;
            
        default:
            sendError("Comando desconocido", "UNKNOWN_CMD");
    }
}

void SerialHandler::sendResponse(const String& json) {
    Serial.println(json);
}

void SerialHandler::sendStatus() {
    MotorState state = motor.getState();
    MotorConfig cfg = motor.getConfig();
    
    String json = "{\"status\":\"ok\",\"msg\":\"Estado actual\",\"data\":{";
    json += "\"running\":" + String(state == MotorState::RUNNING ? "true" : "false");
    json += ",\"direction\":\"" + String(cfg.direction == MotorDirection::INJECT ? "inject" : "aspire") + "\"";
    json += ",\"flow_rate\":" + String(cfg.flow_rate_ml_min);
    json += ",\"microstep\":" + String(cfg.microstep);
    json += "}}";
    
    sendResponse(json);
}

void SerialHandler::sendError(const String& msg, const String& code) {
    String json = "{\"status\":\"error\",\"msg\":\"" + msg + "\",\"code\":\"" + code + "\"}";
    sendResponse(json);
}