#ifndef SERIAL_HANDLER_H
#define SERIAL_HANDLER_H

#include <Arduino.h>
#include "MotorController.h"
#include "CommandParser.h"

class SerialHandler {
public:
    SerialHandler(MotorController& motor);
    
    void begin();
    void loop();
    
private:
    MotorController& motor;
    CommandParser parser;
    String inputBuffer;
    
    void processCommand(const Command& cmd);
    void sendResponse(const String& json);
    void sendStatus();
    void sendError(const String& msg, const String& code);
};

#endif
