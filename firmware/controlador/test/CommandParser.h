#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "MotorController.h"

enum class CommandType {
    NONE,
    CONFIG,
    START,
    STOP,
    STATUS,
    RESET
};

struct Command {
    CommandType type = CommandType::NONE;
    MotorConfig motorConfig;
};

class CommandParser {
public:
    CommandParser();
    
    bool parse(const String& input, Command& cmd);
    String getLastError() const { return lastError; }
    
private:
    String lastError;
    
    bool parseConfig(const JsonObject& obj, MotorConfig& config);
    MotorDirection parseDirection(const String& dir);
};

#endif