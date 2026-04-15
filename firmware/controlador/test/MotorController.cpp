#include "MotorController.h"

MotorController::MotorController() 
    : ioExpander(nullptr)
    , state(MotorState::IDLE)
    , ioExpanderConnected(false)
    , stepInterval_us(0)
    , stepState(false) {
}

bool MotorController::begin() {
    pinMode(PIN_STEP, OUTPUT);
    pinMode(PIN_DIR, OUTPUT);
    digitalWrite(PIN_STEP, LOW);
    digitalWrite(PIN_DIR, LOW);
    
    ioExpander = new PCA9555(PCA9555_ADDR);
    ioExpanderConnected = ioExpander->begin();
    
    if (ioExpanderConnected) {
        ioExpander->pinMode(PCA_ADDR_PIN_EN, OUTPUT);
        ioExpander->digitalWrite(PCA_ADDR_PIN_EN, TMC_ENABLE_OFF);
        Serial.println("[INIT] PCA9555 inicializado");
    } else {
        Serial.println("[INIT] ERROR: PCA9555 no responde");
    }
    
    config.diameter_mm = DEFAULT_DIAMETER;
    config.flow_rate_ml_min = 1.0f;
    config.microstep = DEFAULT_MICROSTEP;
    config.direction = MotorDirection::INJECT;
    
    return true;
}

bool MotorController::configure(const MotorConfig& newConfig) {
    if (newConfig.flow_rate_ml_min <= 0) {
        return false;
    }
    
    config = newConfig;
    stepInterval_us = calculateStepInterval(config.flow_rate_ml_min);
    
    Serial.print("[CONFIG] Flow: ");
    Serial.print(config.flow_rate_ml_min);
    Serial.print(" ml/min, Interval: ");
    Serial.print(stepInterval_us);
    Serial.println(" us");
    
    return true;
}

bool MotorController::start() {
    if (state == MotorState::RUNNING) {
        return false;
    }
    
    if (!ioExpanderConnected) {
        Serial.println("[START] ERROR: PCA9555 no conectado");
        return false;
    }
    
    // Habilitar driver
    ioExpander->digitalWrite(PCA_ADDR_PIN_EN, TMC_ENABLE_ON);
    Serial.println("[START] Driver habilitado");
    
    // Configurar dirección
    if (config.direction == MotorDirection::INJECT) {
        digitalWrite(PIN_DIR, HIGH);
    } else {
        digitalWrite(PIN_DIR, LOW);
    }
    
    stepInterval_us = calculateStepInterval(config.flow_rate_ml_min);
    
    state = MotorState::RUNNING;
    stepState = false;
    
    Serial.print("[START] Motor girando, intervalo: ");
    Serial.print(stepInterval_us);
    Serial.println(" us/step");
    
    return true;
}

bool MotorController::stop() {
    if (state == MotorState::IDLE) {
        return false;
    }
    
    state = MotorState::IDLE;
    
    if (ioExpanderConnected) {
        ioExpander->digitalWrite(PCA_ADDR_PIN_EN, TMC_ENABLE_OFF);
    }
    
    digitalWrite(PIN_STEP, LOW);
    Serial.println("[STOP] Motor detenido");
    
    return true;
}

void MotorController::loop() {
    if (state != MotorState::RUNNING) {
        return;
    }
    
    static uint32_t lastStepTime = 0;
    uint32_t now = micros();
    
    if (now - lastStepTime >= stepInterval_us) {
        stepState = !stepState;
        digitalWrite(PIN_STEP, stepState ? HIGH : LOW);
        lastStepTime = now;
    }
}

uint32_t MotorController::calculateStepInterval(float flow_rate_ml_min) {
    // Calcular pasos por segundo para el flujo deseado
    float radius_mm = config.diameter_mm / 2.0f;
    float area_mm2 = PI * radius_mm * radius_mm;
    float pitch_mm = PITCH_TORNILLO;
    float vol_per_rev_ml = (area_mm2 * pitch_mm) / 1000.0f;
    
    if (vol_per_rev_ml <= 0) {
        return 1000;
    }
    
    float rev_per_sec = flow_rate_ml_min / (vol_per_rev_ml * 60.0f);
    uint32_t steps_per_sec = (uint32_t)(rev_per_sec * STEPS_PER_REV * config.microstep);
    
    if (steps_per_sec == 0) {
        steps_per_sec = 1;
    }
    
    return 1000000UL / steps_per_sec;
}