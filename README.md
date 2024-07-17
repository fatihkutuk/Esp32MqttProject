
# Esp32MqttProject

This repository contains the code for an ESP32 project that connects to an MQTT broker. The project includes detailed instructions for setting up the development environment, obtaining the necessary libraries, and uploading the code to the ESP32 device.

## Table of Contents

- [Introduction](#introduction)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
  - [Arduino IDE Setup](#arduino-ide-setup)
  - [Library Installation](#library-installation)
- [Project Structure](#project-structure)
- [Usage](#usage)
  - [Configuring Wi-Fi and MQTT](#configuring-wi-fi-and-mqtt)
  - [Uploading the Code](#uploading-the-code)
  - [Uploading SPIFFS Data](#uploading-spiffs-data)
  - [Key Points](#KeyPoints)
- [License](#license)
- [Acknowledgements](#acknowledgements)

## Introduction

This project demonstrates how to use an ESP32 microcontroller to connect to an MQTT broker. It includes the necessary code to publish and subscribe to MQTT topics, allowing for real-time data communication between the ESP32 and other MQTT-enabled devices.

## Hardware Requirements

- ESP32 development board
- USB cable for programming
- MQTT broker (e.g., HiveMQ)

## Software Requirements

- Arduino IDE (version 1.8.12 or later)
- ESP32 board package for Arduino IDE
- PubSubClient library for MQTT
- Additional libraries: WiFi, ESPAsyncWebServer, AsyncTCP, DNSServer, SPIFFS, WiFiClientSecure, HTTPClient, ArduinoJson

## Web Server Interface

### Dashboard
![dashboard](https://github.com/user-attachments/assets/b3b3ac1f-4a73-4082-8a41-3fa66a9d19db)
### Wifi Settings
![wifi_settings](https://github.com/user-attachments/assets/99d624f0-c0b6-4e8f-9860-46f87ec4b003)
### Mqtt Settings
![mqtt_settings](https://github.com/user-attachments/assets/e70e394c-5af9-4e64-b86f-3b77f6c40da7)
### Access Point Settings
![ap_settings](https://github.com/user-attachments/assets/6a3be2e5-dd7a-4f3a-b337-75543b4149b1)

## Installation

### Arduino IDE Setup

1. Download and install the [Arduino IDE](https://www.arduino.cc/en/Main/Software).
2. Open the Arduino IDE and go to `File` -> `Preferences`.
3. In the "Additional Board Manager URLs" field, add the following URL:
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
4. Go to `Tools` -> `Board` -> `Boards Manager` and install the "esp32" board package.

### Library Installation

1. Open the Arduino IDE.
2. Go to `Sketch` -> `Include Library` -> `Manage Libraries`.
3. Search for and install the following libraries:
   - **WiFi** (usually included with the ESP32 board package)
   - **ESPAsyncWebServer**: [GitHub link](https://github.com/me-no-dev/ESPAsyncWebServer)
   - **AsyncTCP**: [GitHub link](https://github.com/me-no-dev/AsyncTCP)
   - **DNSServer**: [GitHub link](https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer)
   - **PubSubClient** by Nick O'Leary: [GitHub link](https://github.com/knolleary/pubsubclient)
   - **SPIFFS** (usually included with the ESP32 board package)
   - **WiFiClientSecure** (usually included with the ESP32 board package)
   - **HTTPClient** (usually included with the ESP32 board package)
   - **ArduinoJson**: [GitHub link](https://github.com/bblanchon/ArduinoJson)

### Project Structure

```
Esp32MqttProject
├── .theia
│   ├── settings.json
├── data
│   ├── index.html
│   ├── ap_settings.html
│   ├── mqtt_settings.html
│   ├── wifi_settings.html
│   ├── css
│   │   ├── sb-admin-2.min.css
│   │   ├── all.min.css
│   ├── js
│   │   ├── sb-admin-2.min.js
│   │   ├── jquery.min.js
│   │   ├── bootstrap.bundle.min.js
├── Esp32MqttProject.ino
```

- `.theia/`: Contains IDE-specific settings.
- `data/`: Contains HTML files for the web server.
  - `css/`: Contains CSS files for the web interface.
  - `js/`: Contains JavaScript files for the web interface.
- `Esp32MqttProject.ino`: Main Arduino sketch.

## Usage

### Configuring Wi-Fi and MQTT

1. Open `Esp32MqttProject.ino` in the Arduino IDE.
2. Locate the following lines and update them with your Wi-Fi credentials and MQTT broker details:

   ```cpp
   const char* ssid = "YOUR_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   const char* mqtt_server = "b37.mqtt.one";
   int mqttPort = 1883;
   const char* mqttUser = "89fjvx7161";
   const char* mqttPass = "358acfsvwy";
   ```

### Uploading the Code

1. Connect the ESP32 to your computer using a USB cable.
2. Select the correct board and port in the Arduino IDE:
   - Go to `Tools` -> `Board` and select "ESP32 Dev Module".
   - Go to `Tools` -> `Port` and select the appropriate port.
3. Click the "Upload" button to compile and upload the code to the ESP32.

### Uploading SPIFFS Data

1. Install the [Arduino ESP32 filesystem uploader](https://github.com/me-no-dev/arduino-esp32fs-plugin) by following the instructions on the GitHub page.
2. In the Arduino IDE, go to `Tools` and you should see an option to "ESP32 Sketch Data Upload".
3. Click "ESP32 Sketch Data Upload" to upload the contents of the `data` folder to the ESP32 filesystem.


# Key Points

### Health
 * Reset device after upload data or code with device reset button

### Uploading SPIFFS Data

1. Install the [Arduino ESP32 filesystem uploader](https://github.com/me-no-dev/arduino-esp32fs-plugin) by following these steps:
   - Download the tool from the GitHub repository: [arduino-esp32fs-plugin](https://github.com/me-no-dev/arduino-esp32fs-plugin).
   - Unzip the downloaded file.
   - Copy the extracted `ESP32FS` folder into the `tools` directory of your Arduino IDE installation. The path should look like this:
     ```
     <Arduino Installation Folder>/tools/ESP32FS/tool/esp32fs.jar
     ```
   - Restart the Arduino IDE.

2. In the Arduino IDE, go to `Tools` and you should see an option called "ESP32 Sketch Data Upload".

3. Click "ESP32 Sketch Data Upload" to upload the contents of the `data` folder to the ESP32 filesystem. This step is essential to ensure that the web interface files (HTML, CSS, JS) are available on the ESP32.

### Changeable Mqtt Functions

- The `mqttCallback` function processes incoming MQTT messages. To control the LED on the device, you can send a JSON payload to the control topic:
  - To turn the LED on: `{"deviceled": true}`
  - To turn the LED off: `{"deviceled": false}`

- You can customize the `mqttCallback` function to handle different JSON keys:

  ```cpp
  void mqttCallback(char* topic, byte* payload, unsigned int length) {
      String message;
      for (unsigned int i = 0; i < length; i++) {
          message += (char)payload[i];
      }
      Serial.print("Message arrived [");
      Serial.print(topic);
      Serial.print("]: ");
      Serial.println(message);

      if (String(topic) == controlTopic) {
          DynamicJsonDocument doc(1024);
          DeserializationError error = deserializeJson(doc, message);
          if (error) {
              Serial.print(F("deserializeJson() failed: "));
              Serial.println(error.f_str());
              return;
          }

          if (doc.containsKey("deviceled")) {
              int state = doc["deviceled"];
              digitalWrite(digitalOutput, state ? HIGH : LOW);
              Serial.print("Device LED set to: ");
              Serial.println(state);
          }
          // Add your custom code here to handle other keys
      }
  }
  ```

- The `publishStatus` function sends status updates to the MQTT broker. This function publishes messages to the topic specified in the status topic field on the MQTT settings page:

  ```cpp
  void publishStatus(int digitalInput1State, int digitalInput2State) {
      String payload = "{"digital_input_1":" + String(digitalInput1State) + ","digital_input_2":" + String(digitalInput2State) +
                       ","local_ip":"" + localIP + "","ap_ip":"" + apIPStr + "","external_ip":"" + externalIP + ""}";
      client.publish(statusTopic, payload.c_str());
      Serial.println("Status published: " + payload);
  }
  ```

- The `mqttCallback` function listens for messages on the control topic defined in the MQTT settings page.
- The `publishStatus` function publishes messages to the status topic defined in the MQTT settings page.


## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgements

- [Arduino](https://www.arduino.cc/)
- [ESP32](https://www.espressif.com/en/products/socs/esp32)
- [PubSubClient](https://pubsubclient.knolleary.net/)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)
- [DNSServer](https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer)
- [ArduinoJson](https://arduinojson.org/)


