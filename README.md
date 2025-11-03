# Technologies for Big Data Management – IoT Lab

Hands-on lab material for the *Technologies for Big Data Management* master course.  
The lab walks through building a simple sensing node with an Arduino Uno (or Grove Base Shield compatible board) using the Grove Starter Kit v3. The node samples **temperature**, **ambient light**, and **sound**, prints measurements on the RGB LCD, and streams values over the serial port (optionally via the bundled Bluetooth module).

## Repository Layout

- `arduino_code/arduino_code.ino` – starter sketch for the Arduino/Grove kit exercise (focus of this README).
- `esp32-code/` – companion PlatformIO project showing one possible gateway implementation (ESP32 + MQTT). Use it only after completing the Arduino part.

## Prerequisites

- Hardware:
  - Arduino Uno (or Seeed Wio/Grove-compatible board) with Grove Base Shield.
  - Grove RGB LCD (16x2) display.
  - Grove Temperature Sensor (classic NTC thermistor).
  - Grove Light Sensor v1.2.
  - Grove Sound Sensor.
  - Grove cables, USB cable, external 5V supply (if needed).
- Software:
  - Arduino IDE ≥ 2.x or Arduino CLI.
  - Board support package for Arduino Uno.
  - Libraries (install from Arduino Library Manager):
    - **Grove LCD RGB Backlight** (`rgb_lcd.h`).
    - **SoftwareSerial** (bundled with Arduino AVR boards).

## Wiring Cheat Sheet

| Grove Module          | Shield Port | Arduino Pin Mapping | Notes                              |
|-----------------------|-------------|---------------------|------------------------------------|
| RGB LCD (I2C)         | I2C-1 or 2  | SDA → A4, SCL → A5  | Needs 5V. Chainable on I2C ports.  |
| Temperature Sensor    | A0          | Analog A0           | Uses 10 kΩ thermistor (`B=3975`).  |
| Light Sensor          | A1          | Analog A1           | Outputs higher values in bright light. |
| Sound Sensor          | A2          | Analog A2           | Raw amplitude envelope. Tune delays if noisy. |
| (Optional) BT Module  | D10/D11     | SoftwareSerial RX/TX| Uncomment lines in sketch when used. |

> Tip: keep the analog cables short and avoid bending them sharply—the sound sensor is sensitive to interference.

## Arduino Sketch Walkthrough

The starter sketch already wires up the LCD and exposes helper functions for each sensor:

- `convert_temperature()` converts the raw ADC reading using the thermistor Beta formula.
- `read_temperature()`, `read_light()`, and `read_sound()` are intentionally incomplete – fill in the TODO blocks with actual `analogRead()` calls and reuse `convert_temperature()`.
- `print_info()` clears the LCD and shows temperature, light, and sound levels so you can confirm the node is working without a PC.
- The main `loop()` polls every 5 seconds, sends debug traces to the Serial Monitor, and prints the values on screen. Tweak `delay(5000)` if you need faster updates.

Once the TODOs are filled, you should see output similar to:

```
....
T.:23.7  L.:526
Snd:118
```

If you attach the Grove Bluetooth module, uncomment the `SoftwareSerial` declarations and the `send_data_as_json()` helper to stream JSON payloads to an external receiver (e.g., ESP32 gateway or laptop).

## Build & Upload

1. Open `arduino_code/arduino_code.ino` in the Arduino IDE.
2. Select **Arduino Uno** (or your board) and the correct serial port.
3. Install missing libraries via **Tools → Manage Libraries…** if the compiler reports them.
4. Verify (`⌘R` / `Ctrl+R`) and upload (`⌘U` / `Ctrl+U`).
5. Use the Serial Monitor at 9600 baud to inspect the debug prints while watching the LCD.

## Validation Steps

- Cover/uncover the light sensor: LCD light value should jump.
- Warm the temperature sensor with your hand or ice spray: the temperature line should drift accordingly.
- Clap near the sound sensor: the sound reading spikes.
- (Optional) Capture serial data with a Python script, Arduino Serial Plotter, or a BLE terminal to feed the ESP32 gateway in `esp32-code/`.

## Extension Ideas

1. Replace fixed sampling with a moving average or rolling window to smooth sound readings.
2. Raise LCD alerts (e.g., change background color) when thresholds are crossed.
3. Package measurements as JSON on the serial line and forward them to a cloud MQTT broker with the ESP32 bridge.

Document your wiring, calibration notes, and any code changes for grading—this README is meant to be the starting point for your lab report.
