# ##############################################################################
# A2170/BQ30Z55 ESP32 SMBus Battery Monitor Dashboard
# ##############################################################################

This repository contains the firmware and layout configurations to build a **real-time Wi-Fi Battery Monitor Web Dashboard** for smart laptop and custom lithium-ion battery packs using an **ESP32 microcontroller** and **SMBus/I2C communication**.

Designed specifically for packs utilizing the **A2170 OEM battery management integrated circuit (BMS IC)**—which is programmed to emulate the standard **Texas Instruments BQ30Z55** fuel gauge family—this code queries standard Smart Battery System (SBS) registers to fetch pack parameters.

---

## ⚡ Hardware Wiring Configuration

To connect your battery pack to the ESP32, connect the clock, data, and ground lines. Because SMBus operates with active-low signals, you **must include 4.7 kΩ pull-up resistors** connecting the `SDA` and `SCL` lines to the ESP32's `3.3V` rail.

```
       ESP32 Pinout                             A2170 Battery Pack
     +--------------+                         +---------------------+
     |          3.3V|------+---[ 4.7k ]---+   |                     |
     |              |      |              |   |                     |
     |        GPIO21|------+------------->|-->| SDA (Pin 8 / Data)  |
     |        GPIO22|----------[ 4.7k ]+  |   |                     |
     |              |                  |  |   |                     |
     |              |------------------+->|-->| SCL (Pin 7 / Clock) |
     |           GND|--------------------->|-->| GND (System Ground) |
     +--------------+                         +---------------------+
```

*Note: For laptop packs like the **HP CS03XL (HSTNN-DB6U)**, verify your pinout diagram to ensure you connect correctly to the battery pack terminals.*

---

## 📁 Repository Structure

* **`a2170-esp32-monitor.ino`**: The main Arduino sketch. It configures the ESP32 as a local **Wi-Fi Access Point (AP)**, starts a web server, handles the repeated-start SMBus transaction layer, and generates a dynamically refreshed web page dashboard.
* **`LICENSE`**: Open-source license (MIT).

---

## 🚀 Getting Started

### 1. Requirements
* **Hardware**:
  * ESP32 Development Board (e.g., NodeMCU ESP32)
  * Smart battery pack containing an A2170 or BQ30Z55 IC
  * Two 4.7 kΩ resistors (pull-up resistors)
  * Jumper wires
* **Software**:
  * [Arduino IDE](https://www.arduino.cc/en/software)
  * ESP32 Board Core (installed via Arduino Board Manager)

### 2. Installation & Upload
1. Clone this repository to your local machine:
   ```bash
   git clone https://github.com/Rajgauravyadav1/esp32-a2170-battery-monitor
   ```
2. Open `a2170-esp32-monitor.ino` in the Arduino IDE.
3. Configure your Arduino IDE settings:
   * Board: **ESP32 Dev Module** (or your specific ESP32 variant)
   * Port: Select the COM port connected to your ESP32
4. Click **Upload** to flash the code to your ESP32.

---

## 📡 Usage

1. Once uploaded, open the Serial Monitor (Baud Rate: `115200`).
2. The ESP32 will start an Access Point with the following credentials:
   * **SSID**: `BatteryMonitor`
   * **Password**: `12345678`
3. Connect your computer or smartphone to the **`BatteryMonitor`** Wi-Fi network.
4. Open your web browser and navigate to **`http://192.168.4.1`** (or the IP displayed in the Serial Monitor).
5. The dashboard will automatically load and **refresh every 2 seconds** with live telemetry!

---

## 🔬 SMBus Register & Communication Insights

### Individual Cell Voltages Count Downward
Standard Smart Battery System (SBS) registers for individual cell voltages count **downward** from `0x3F`:
* **`0x3F`**: Cell 1 Voltage
* **`0x3E`**: Cell 2 Voltage
* **`0x3D`**: Cell 3 Voltage

This firmware handles this countdown logic dynamically based on the configured `NUM_CELLS` parameter.

### FET Charging/Discharging Status
Unlike standard logic, the physical states of the Charge (CHG) and Discharge (DSG) MOSFETs on the A2170/BQ30Z55 are monitored using the **`OperationStatus()`** register at command code **`0x54`**. This firmware parses the 16-bit status word:
* **DSG MOSFET State**: Bit 2 (`0x0004`)
* **CHG MOSFET State**: Bit 1 (`0x0002`)

---

## ⚠️ Safety Warning & Disclaimer

> **DANGER**: Working with lithium-ion/polymer batteries carries inherent risks of short-circuits, thermal runaway, smoke, fire, and explosions. Always double-check your wiring. Never apply high voltages to the SMBus pins. Use appropriate fuses and thermal protection mechanisms in your system designs.
