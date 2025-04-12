# Infusion Pump Project

An open-source, DIY infusion pump system designed for precision fluid handling in chemistry and laboratory applications.

## Overview

This repository contains all the necessary components to build your own programmable infusion pump system, including 3D models for printing the mechanical components, PCB designs for the electronics, and firmware for controlling the system.

## Repository Structure

- **3d_models/** - 3D printable files for the physical components
  - **pump/** - Main pump components
  - **tube_connectors/** - Components for fluid connections

- **electronics/** - PCB designs and schematic files
  - **power_distribution_board/** - Power supply and distribution circuits
  - **pump_main_board/** - Main control board for the pump

- **firmware/** - Software that runs on the pump hardware
  - **syringe_pump_master/** - Controller firmware for the pump system

## Features

- Precision fluid control
- Programmable flow rates
- Compatible with standard syringes and tubing
- Modular design for easy customization
- Open-source hardware and software

## Getting Started

### Hardware Requirements

- 3D printer for fabricating components, I use and recommend PETG material
- Electronic components: Seeed Studio Xiao ESP32C3, TMC2209 module, 1Kohm resistors (THT or 0805 SMD), LED, 4p 2.54mm JST connector, 6p 2.54mm JST connector, 7p pin sockets, 8p pin sockets, 12mm M3 screws, 16mm M3 screws, M3 nuts, m3 washers
- Tools for assembly: Screwdriver, soldering iron, solder
- Wifi network

### Building the Pump

1. Print the 3D models from the `3d_models` directory
2. Manufacture PCBs using files in the `electronics` directory
3. Assemble the hardware components
4. Flash the firmware to the microcontroller

### Software Setup

Use Arduino IDE to upload firmware to microcontroller Seeed Studio Xiao ESP32C3. Make sure to set mqtt_server IP, unique device_name, WiFiClient and PubSubClient client (set in syringe_pump_master.ino)

## License

This project is open-source and available under the MIT License.

## Contributing

Contributions to improve the design, firmware, or documentation are welcome. Please feel free to submit pull requests or open issues for discussion. 