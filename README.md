# Sphere Grouop - Sphere Drones Motherboard V3 PCB

## Introduction

This code is a new implementation of the embedded code running on the HUB-X and HUB-T devices. It has been re-written from the ground up to deal with the ongoing needs, and with some view for the future. 

## Architecture

The code is written for the STM32F4xx series processors, using PlatformIO as the programming architecture. This provides the simplicity of the Arduino architecture whilst permitting a more structured programming environment. PlatformIO allows different code versions to be compiled based on the same code, with either different variants of the product, or for different processors. This allows the same code base to be used for different versions of the underlying hardware.

## Capabilities

This code runs with companion hardware. The core I/O includes:

* Digital temperature monitoring
* Voltage switching of outputs
* Current and voltage monitoring
* Fan PWM control with tacho speed monitoring - with overriding power control using voltage switching
* LED Controls
* Lock control outputs
* Modbus expansion via RS485
* Air conditioner control

Communications includes: 

* ESP32 expansion for MQTT over ethernet
* On board ethernet controller
* Serial expansion using the ESP32 serial poer, when the ESP32 itself is depricated. 


# Sensors

## Temperature

The software monitors eight Dallas OneWire strings for temperatrue sensors. 
