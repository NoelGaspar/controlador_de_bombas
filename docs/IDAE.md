# Controlador_Bombas

Este proyecto considera un controlador sencillo de bombas. Esta basado en una esp32 el cual controla vía comandos un motor stepper que acciona una jeringa por medio de un mecanismos de tornillos sin fines. El sistema recibe los comandos desde el computador (host) y genera las acciones necesarias para controlar el sentido de giro, los microstepping entre otros. 

# Función principal

Este dispostivo funciona en una placa PCB de diseño pripio que integra tanto una ESP32 como toda la electrónica necesaría para la gestión de consumo energético del motor como la integración del driver. 

El driver utilizado es el driver TMC2226 y cuenta con la posibilidad de enviarle comandos vía UART, los cuales se usarán para configurar el driver. Para su operación el sistema debe considerar las señales EN, STEP y DIR

Para los pines de control se utilizaron las siguientes asignaciones de pines:

Driver TMC2226  - ESP32
EN              - io15 (expansor PCA9555)
MS1				- io14 (expansor PCA9555)
MS2				- io13 (expansor PCA9555)
PND				- GPIO16
PND				- GPIO17 (vía resistencia de 1k)
CLK				- NC
STEP 			- GPIO4
DIR  			- GPIO23



La tarea principal es mover el motor de forma controlada basandonos principalmente en el flujo mas que en la cantidad de agua transportada. Es por esto que el usuario debe ingresar los parametros necesarios como diametro de la jeringa, flujo deseado, tipo de reducción (si es que existe, asumir 1:1 por defecto), tipo de acción (aspiración o inyección) y microstepping utilizado.  Luego enviar el comando start para que empiece a funcionar y posteriormente el comando stop para detener el funcionamiento del motor.

Es ideal que los comandos se gestionen todos en formato json y el dispositivo pueda ir entregando mensajes de status para debug. 

La idea es eventualmente extender el sistema para que el dispositvio logre gestionar 4 motores de forma simultanea. Para evitar el sobre consumo de pines disponibles de la esp32, la pcb cuenta con un extensor de io ports configurado por i2c. El extensor es el PCA9555 su address i2c 0x20

Para mejorar la robustez del proyecto sería ideal diseñar elcódigo basado en task y contar con un task para controlar el motor y otro task para gestionar los comandos por uart. 

## Fases de implementación 

Abordaremos el problema desde dos etapas.

La etapa 1 considera la estructura de comandos básicos de operación y valida el hardware. Es decir, vamos a implementar todas las funciones que permitan el movimiento del motor y la configuración de los parámetros. 

La etapa 2 considera una gui, un mejor control del dispositivo y la calibración para su funcionamiento. 

## Herramientas

- Las principales herramientas se describen en el archivo AGENTS.md.
- El proyecto estará alojado en el siguiente repositorio : https://github.com/NoelGaspar/controlador_de_bombas 

## Bibliotecas

Las bilbiotecas utilizadas son:

- clsPCA9555.h        (para gestionar el extensor de puertos)
- ContinuousStepper.h ( para controlar el stepper por velocidad de giro, RPM y no por posición)
- Preferences.h 	(Para guardar los parametros configurados en la sesión anterior)
- ArduJson 			(Para gestionar los comandos por json)