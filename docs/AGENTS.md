# AGENTS.md

## Contexto del Proyecto
Este es un proyecto de desarrollo tecnológico en el area de instrumentación cientifica que integra firmware, software y hardware. Basado en una ESP32.

## Firmware y software
- El proyecto considera que el firmware desarrollado en platformIO en el framerwork de Arduino
- Los códigos de software (test y  gui) deben estar escritos en python
- Los códigos de software desarrollados en python deben usar entornos virtuales con venv
- El firmware debe considerar estructuras modulares, priorizando la creación de clases generales y bibliotecas de las funcionalidades del proyecto en archivos .h y .cpp
- El firmware debe considerar un control de versiones. almacenado en una variable dentro del código.

## Debug y test

- Los debug deben realizarse vía UART por serial y registrar los resultados de los test en python. 



## Estructura del directorios

controlador_de_bombas/
├── docs/
│   ├── IDEA.md          (tú lo crearás)
│   ├── AGENTE.md        (tú lo crearás)
│   ├── FSD.md           (yo lo crearé)
│   ├── LOGS.md          (para decisiones de diseño)
│   └── datasheets/
│       └── Archivo PDF con los datashets de los componentes
├── firmware/ 
│   └── controlador/         (Proyecto de Platformio)
│       └── src/
│               main.cpp     (código main para ESP32)
├── software/
│   ├── client.py        (cliente CLI Python)
│   ├── tests.py         (validación)
│   └── logs/
│       └──test_log.txt  (resultado output de cada test)
└── README.md
