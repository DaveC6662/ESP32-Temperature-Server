# ESP32 Temperature Monitoring and Web Server

## Overview
This project uses an ESP32 board to monitor temperature with a DS18B20 sensor. It provides a web interface for real-time temperature data display and configuration settings. The system also supports alerting through webhooks for specified temperature thresholds.

## Features
- Real-time temperature monitoring with DS18B20 sensor
- Web server for data display and settings configuration
- Temperature threshold alerting via webhooks
- WiFi connectivity with support for WPA2-Personal and WPA2-Enterprise
- Dynamic web content with live temperature updates

## Wifi Connection
![IMG_1600](https://github.com/DaveC6662/ESP32-Temperature-Server/assets/141587948/cdce02ec-40fc-4331-85bb-3317f62f273b)

## Captive Portal Home Page
![IMG_1601](https://github.com/DaveC6662/ESP32-Temperature-Server/assets/141587948/f58a008f-c206-45fa-b963-3186f5dcabad)

## Captive Portal Security Selection
![IMG_1602](https://github.com/DaveC6662/ESP32-Temperature-Server/assets/141587948/8c5279d7-b197-4ef5-b05d-d3328850dead)

## Captive Portal WPA2 Personal
![IMG_1603](https://github.com/DaveC6662/ESP32-Temperature-Server/assets/141587948/c97a4795-5d4b-4aa1-a4cb-d3fdc9823d5a)

## Captive Portal WPA2 Enterprise
![IMG_1604](https://github.com/DaveC6662/ESP32-Temperature-Server/assets/141587948/a0761aa9-1013-4101-9b7f-1307c98a611e)

## Temperature Server Home Page
![MainServer](https://github.com/DaveC6662/ESP32-Temperature-Server/assets/141587948/74848521-150f-431f-869d-4f5895d9bb9d)

## Temperature Server Settings Page
![ServerSettings](https://github.com/DaveC6662/ESP32-Temperature-Server/assets/141587948/c33c21fb-a1dd-4c1a-ae57-9ca1e24418a6)

## Hardware Requirements
- ESP32 board
- DS18B20 temperature sensor
- 4.7k resistor

## Software Dependencies
- [Arduino IDE](https://www.arduino.cc/en/Main/Software)
- [DNSServer](https://www.arduino.cc/en/Reference/DNSServer) for setting up a DNS server
- [WiFi](https://www.arduino.cc/en/Reference/WiFi) for managing WiFi connections
- [AsyncTCP](https://github.com/me-no-dev/AsyncTCP) as a dependency for ESPAsyncWebServer
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) for creating an asynchronous web server on the ESP32
- [OneWire](https://www.arduinolibraries.info/libraries/one-wire) for communication with OneWire devices like the DS18B20 temperature sensor
- [DallasTemperature](https://www.milesburton.com/Dallas_Temperature_Control_Library) for interfacing with the DS18B20 temperature sensors
- [esp_wpa2](https://www.arduino.cc/en/Reference/WiFiBeginEnterprise) for WPA2 Enterprise WiFi security
- [HTTPClient](https://www.arduino.cc/reference/en/libraries/httpclient/) for sending HTTP requests
- [ArduinoJson](https://arduinojson.org/) for JSON parsing and serialization

## Installation
1. Install the Arduino IDE and the required libraries.
2. Clone or download this repository.
3. Open the project in Arduino IDE.
4. Update the `secrets.h` file with your desired passcode and webhook details.
5. Compile and upload the code to the ESP32 board.

## Configuration
- On first boot, connect to the ESP32's WiFi network for initial setup.
- Access the web server using the ESP32's IP address and a device on the same network.
- Set the desired temperature thresholds through the web interface.

## Usage
- The web interface displays live temperature readings.
- Configure alert thresholds to receive notifications via webhooks.
- Use the `/data` and `/info` endpoints for JSON formatted data.

## API Endpoints
- `/data`: Returns temperature data in JSON format.
- `/info`: Provides device and connection information.

## Security
- Handle WiFi credentials and webhook URLs securely.
- Use SSL certificates for secure HTTP connections.

## Contributing
Contributions are welcome. Please adhere to coding standards and include tests and documentation with your contributions.

## License
This project is licensed under the Apache 2.0 License - see the [LICENSE.md](LICENSE.md) file for details.
