#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "config.h"
#include <clsPCA9555.h>

enum class MotorDirection {
    INJECT,
    ASPIRE
};

enum class MotorState {
    IDLE,
    RUNNING
};

struct MotorConfig {
    float diameter_mm = DEFAULT_DIAMETER;
    float flow_rate_ml_min = 1.0f;
    uint16_t microstep = DEFAULT_MICROSTEP;
    MotorDirection direction = MotorDirection::INJECT;
};

class MotorController {
public:
    MotorController();
    
    bool begin();
    bool configure(const MotorConfig& config);
    bool start();
    bool stop();
    void loop();
    
    MotorState getState() const { return state; }
    MotorConfig getConfig() const { return config; }

private:
    PCA9555* ioExpander;
    MotorState state;
    MotorConfig config;
    bool ioExpanderConnected;
    
    uint32_t stepInterval_us;
    bool stepState;
    
    uint32_t calculateStepInterval(float flow_rate_ml_min);
};

#endif