# Wireless Embedded System for Image Capture, Data Logging and Actuation

**Bachelor's Thesis (TFG)**  
Degree in Industrial Electronics and Automation Engineering  
Universidad Politécnica de Cartagena (UPCT) · July 2024

**Author:** Antonio Joaquín Piñera Marín

---

## Overview

Design and implementation of a low-power wireless embedded system for IoT environments. The system integrates a **LoRa Gateway with NB-IoT connectivity** and an **image capture node**, capable of collecting sensor data, transmitting it over LoRa, pushing it to a cloud IoT platform, and storing images remotely. The project was validated in a real outdoor deployment.

---

## System Architecture

```
[LoRa sensor node]
        │  LoRa (radio)
        ▼
[Gateway — Heltec WiFi LoRa32 (V2)]
   ├── NB-IoT (SIM7020-E) ──────► Thingspeak (sensor data)
   └── [ESP32-CAM] ─── WiFi ───► Firebase Storage (images)
```

The system operates autonomously powered by a solar panel and lead-acid battery.

---

## Hardware

| Component | Role |
|---|---|
| Heltec WiFi LoRa32 (V2) | Main MCU + LoRa radio + OLED display |
| ESP32-CAM + OV2640 | Image capture and upload (up to 2 MP) |
| SIM7020-E | NB-IoT connectivity (M2M) |
| Solar panel + Lead-acid battery | Autonomous outdoor power supply |
| Buck converter MP1584 (DC-DC) | Voltage regulation |
| Perfboard | Final assembly and soldering |

---

## Software

The firmware is split into two independent modules, both developed in **C/C++ with Arduino IDE 2.0**:

### `camera/`
ESP32-CAM node firmware. Handles:
- OV2640 camera module initialisation and configuration
- Periodic image capture
- Authentication and upload to **Firebase Storage** over WiFi
- Low-power modes (*deep sleep*)

### `gateway/`
Heltec WiFi LoRa32 (V2) Gateway firmware. Handles:
- LoRa frame reception from remote sensor nodes
- Data forwarding to **Thingspeak** via the NB-IoT SIM7020-E module (AT commands over UART)
- System status display on the OLED screen
- Coordination with the ESP32-CAM module

---

## Communication Protocols

- **LoRa / LoRaWAN** — long-range, low-power radio link between nodes and Gateway
- **NB-IoT** — cellular connectivity for cloud data upload (SIM7020-E via AT commands / UART)
- **WiFi** — image transfer from ESP32-CAM to Firebase
- **SPI** — interface with the LoRa radio module
- **I²C** — OLED display on the Heltec board
- **UART** — main MCU ↔ SIM7020-E communication

---

## Cloud Platforms

- **[Thingspeak](https://thingspeak.com)** — time-series storage and visualisation of sensor data
- **[Firebase Storage](https://firebase.google.com)** — image storage with authentication and real-time access

---

## Results

- Functional LoRa transmission between node and Gateway in an outdoor environment
- Image capture at up to **2 megapixels** with clear visual quality
- Sensor data available on Thingspeak in near real-time after reception
- Images stored on Firebase immediately after capture
- System validated in a real-world deployment (outdoor garden)

---

## Repository Structure

```
├── camera/           # ESP32-CAM firmware (C/C++)
├── gateway/          # LoRa + NB-IoT Gateway firmware (C/C++)
├── hardware/         # KiCad Schematics (MCU, power supply, Buck converter, SIM7020-E, CAM)
└── README.md
```

---

## Tech Stack


**MCU:** ESP32 ( ![Heltec WiFi LoRa32 V2](https://heltec.org/project/wifi-lora-32v2), ![ESP32-CAM](https://tienda.bricogeek.com/arduino-compatibles/1912-esp32-cam-esp32-con-camara-ov2640.html))  
**Protocols:** LoRa, NB-IoT, WiFi, SPI, I²C, UART  
**Cloud:** ![Firebase](https://firebase.google.com/), ![Thingspeak](https://thingspeak.mathworks.com/)  
**Languages:** C / C++  
**IDE:** ![Arduino IDE 2.0](https://docs.arduino.cc/software/ide-v2/tutorials/getting-started/ide-v2-downloading-and-installing/)
**Schematics:** ![KiCad](https://www.kicad.org/download/)

---

## Future Work

- PCB design to reduce footprint, improve robustness and power efficiency
- Computer vision integration (intrusion detection, facial recognition)
- Mobile application development connected to Firebase
- End-to-end data encryption for improved security
- Unification of all communications into a single radio module (WiFi or M2M)

---

## License

Academic project — UPCT 2024. All rights reserved by the author.
