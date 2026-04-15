#include "CommandParser.h"

CommandParser::CommandParser() {
}

bool CommandParser::parse(const String& input, Command& cmd) {
    cmd.type = CommandType::NONE;
    
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, input);
    
    if (error) {
        lastError = String("JSON parse error: ") + error.c_str();
        return false;
    }
    
    JsonObject obj = doc.as<JsonObject>();
    if (!obj.containsKey("cmd")) {
        lastError = "Falta campo 'cmd'";
        return false;
    }
    
    String command = obj["cmd"].as<String>();
    
    if (command == "config") {
        cmd.type = CommandType::CONFIG;
        return parseConfig(obj, cmd.motorConfig);
    }
    else if (command == "start") {
        cmd.type = CommandType::START;
        return true;
    }
    else if (command == "stop") {
        cmd.type = CommandType::STOP;
        return true;
    }
    else if (command == "status") {
        cmd.type = CommandType::STATUS;
        return true;
    }
    else if (command == "reset") {
        cmd.type = CommandType::RESET;
        return true;
    }
    
    lastError = "Comando desconocido: " + command;
    return false;
}

bool CommandParser::parseConfig(const JsonObject& obj, MotorConfig& config) {
    if (obj.containsKey("diameter")) {
        config.diameter_mm = obj["diameter"].as<float>();
    }
    
    if (obj.containsKey("flow_rate")) {
        config.flow_rate_ml_min = obj["flow_rate"].as<float>();
    }
    
    if (obj.containsKey("microstep")) {
        config.microstep = obj["microstep"].as<uint16_t>();
    }
    
    if (obj.containsKey("direction")) {
        String dir = obj["direction"].as<String>();
        config.direction = parseDirection(dir);
    }
    
    return true;
}

MotorDirection CommandParser::parseDirection(const String& dir) {
    if (dir.indexOf("inject") >= 0) {
        return MotorDirection::INJECT;
    }
    return MotorDirection::ASPIRE;
}