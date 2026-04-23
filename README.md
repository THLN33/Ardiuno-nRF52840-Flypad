# nRF52840 BLE Central: Parrot Flypad Controller Integration

This project demonstrates how to use an **nRF52840** (Pro Micro clone / SuperMini) as a **Bluetooth LE Central** to interface with a **Parrot Flypad** controller using the Arduino IDE.

## 🚀 Overview

The goal is to intercept joystick and button data from the Parrot Flypad and process them on the nRF52840. This is particularly useful for building custom drones, RC cars, or robotic platforms that require a high-quality physical remote.

### Key Features
* **BLE Central Mode**: Automatically scans, identifies, and connects to the Flypad.
* **Service Discovery**: Explores GATT services (Battery, Device Info, and Parrot Custom Service).
* **Real-time Notifications**: Subscribes to the Flypad data characteristic for low-latency input handling.
* **Arduino Compatible**: Based on the Adafruit Bluefruit nRF52 library.

---

## 🛠 Hardware Requirements

1.  **nRF52840 Board**: (e.g., Nice!Nano, SuperMini nRF52840, or SparkFun Pro Micro nRF52840).
2.  **Parrot Flypad**: The BLE controller originally designed for Minidrones.

---

## 🏗 Installation & Setup

### 1. Bootloader Update
Before using the Arduino IDE, you must flash the **Adafruit nRF52 Bootloader**. 
This project follows the excellent guide by **BeachyUK**:
👉 [Connecting and Testing Pro Micro nRF52840 Clones](https://www.beachyuk.com/blog/connecting-and-testing-promicro-nrf52840-clones)

### 2. Software Dependencies
Install the following in your Arduino IDE:
* **Board Support**: `nRF52 by Adafruit` (via Boards Manager).
* **Library**: `Adafruit Bluefruit nRF52` by Adafruit.

---

## 💻 Code Architecture

The implementation uses a callback-based architecture to ensure efficiency:

* **`scan_callback`**: Filters BLE advertising packets to find the Parrot Service UUID.
* **`connect_callback`**: Triggered when the handshake is successful.
* **`list_services`**: Discovers the specific Parrot GATT characteristics.
* **`data_callback`**: The core function where raw bytes from joysticks/buttons are received and printed to the Serial Monitor.

### GATT UUIDs used
| Service / Char | UUID (Little Endian in code) |
| :--- | :--- |
| **Parrot Service** | `9E35FA00-4344-44D4-A2E2-0C7F6046878B` |
| **Data Characteristic** | `9E35FA01-4344-44D4-A2E2-0C7F6046878B` |

---

## 🤝 Acknowledgments

* [BeachyUK](https://www.beachyuk.com/) for the bootloader flashing tutorial.
* [Adafruit](https://www.adafruit.com/) for the Bluefruit nRF52 library.
* [pdcook](https://github.com/pdcook) for the nRFMicro-like-boards support.

---

## 📝 License
This project is open-source. Feel free to use and modify it for your own robotic projects!
