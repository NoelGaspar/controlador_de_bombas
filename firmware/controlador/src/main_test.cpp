#include <Arduino.h>
#include <Wire.h>
#include "clsPCA9555.h"
#include <ContinuousStepper.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <TMCStepper.h>

#define FIRMWARE_VERSION "1.0.0"

// Debug Settings
#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL 0  // Set to 1 to enable verbose debug messages
#endif

#define GPIO_ADDR 0x20  // PCA9555 I/O Expander

// Stepper motor steps per revolution (1.8° per step = 200 steps/rev)
#define STEP_PER_REV 200
#define RXD2 17
#define TXD2 16
#define DRIVER_ADDRESS 0b00 // TMC2209 address pins (MS1, MS2) tied to GND
#define R_SENSE          0.11f


PCA9555 ioport(0x20);
Preferences preferences;

// En ESP32 usa HardwareSerial, no SoftwareSerial
HardwareSerial TMCSerial(2);

TMC2209Stepper driver(&TMCSerial, R_SENSE, DRIVER_ADDRESS);
ContinuousStepper<StepperDriver> stepper;


int microstep_value = 2;    
long last_milis = 0;


void printDriverStatus() {
    Serial.println("---- TMC2209 STATUS ----");
  
    // Estas lecturas sirven para saber si UART está viva
    Serial.print("IOIN: 0x");
    Serial.println(driver.IOIN(), HEX);
  
    Serial.print("GSTAT: 0x");
    Serial.println(driver.GSTAT(), HEX);
  
    Serial.print("Microsteps config requested: ");
    Serial.println(microstep_value);
  
    // MSCNT cambia con el movimiento, útil para ver vida
    Serial.print("MSCNT: ");
    Serial.println(driver.MSCNT());
  
    Serial.println("------------------------");
}


void setup() {
    
    
    Serial.begin(115200);
    Wire.begin();
    delay(500);

  
    ioport.begin();
    ioport.pinMode(15, OUTPUT); // Enable pin for TMC2226
    ioport.pinMode(14, OUTPUT); // Enable pin for TMC2226
    ioport.pinMode(13, OUTPUT); // Enable pin for TMC2226
    ioport.digitalWrite(15, LOW); // Disable motor on startup         
    ioport.digitalWrite(13, LOW);
    ioport.digitalWrite(14, LOW);

      // UART hardware del ESP32
     TMCSerial.begin(115200, SERIAL_8N1, RXD2, TXD2);
     delay(100);

    driver.begin();             // Initialize driver
   // Muy importante para usar UART en vez de la config por pin
   driver.pdn_disable(true);
   driver.mstep_reg_select(true);
 
   driver.toff(5);
   driver.rms_current(600);
   driver.microsteps(16);   // prueba inicial simple
   driver.I_scale_analog(false);
 
   delay(50);
 
   printDriverStatus();

    stepper.begin(/*step=*/4, /*dir=*/23);
    stepper.spin(400);
}

void loop() 
{
    if (millis() -last_milis> 10000) 
    {
        stepper.spin(0);
        delay(200);

        microstep_value *= 2;
        if (microstep_value > 256) microstep_value = 2;
        
        driver.microsteps(microstep_value); 
        delay(50); // Pequeña pausa para asegurar que el driver procese el cambio de microstepping

        Serial.print("Microsteps set to: ");
        Serial.println(microstep_value); 

        printDriverStatus();
        stepper.spin(400);
        
        last_milis = millis();
        
    }
   stepper.loop();
   /*
   delay(30000);
   stepper.stop();
   driver.microsteps(32);
   stepper.spin(200);

   delay(30000);
   stepper.stop();
   driver.microsteps(64);
   stepper.spin(200);
   
   delay(30000);
   stepper.stop();
   driver.microsteps(128);
   stepper.spin(200);
   

   delay(30000);*/
}