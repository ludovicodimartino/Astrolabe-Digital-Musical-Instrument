# Astrolabio project

Research training project for the Master's Degree Program in Computer Engineering. The topic of the activity is the design and development of the digital musical instrument based on the astrolabe from the [_Giovanni Poleni_ museum](https://www.musei.unipd.it/it/fisica#) in Padua for the show _Caravanserraglio_.

## Project Structure

```
Astrolabio project
│
└───Arduino Sketches                   # The developed sketches for the Arduino board
│
└───Controller Design                  # The designed parts for building the physical controller
│
└───Digital Twin Desktop Application   # Electron app for viewing the 3D astrolabe model
│
└───Fritzing Breadboard Diagrams       # The design of the breadboards for the first and second design iteration
│
└───PureData Patches                   # The developed Pure Data patches
│   │
│   └───abstractions                   # Reusable Pure Data Objects for reading the sensor data from the network
│   │
│   └───tests                          # Patches for testing the communication with the Arduino
│
└─── README.md
```


## Hardware Components

For the final prototype, the following components were used:

-   [ESP32-C3 Super Mini Dev Board](https://www.espboards.dev/esp32/esp32-c3-super-mini/)
-   [Adafruit Precision NXP 9-DOF Breakout Board - FXOS8700 + FXAS21002](https://www.adafruit.com/product/3463)
-   [Dual rotary encoder](https://docs.rs-online.com/f0a8/A700000009887456.pdf)
-   [LiPo Rider Plus](https://wiki.seeedstudio.com/Lipo-Rider-Plus/)
-   LiPo Battery 3.7V 1800mAh

For the communication between the microcontroller and the IMU (Adafruit Precision NXP 9-DOF), the I<sup>2</sup>C serial communication system was used. Here’s how everything was wired:
![image](./Fritzing%20Breadboard%20Diagrams//ESP32C3Breadboard.png)

## How to run the Arduino DMI code on the ESP32-C3 Dev board

1.  In the Arduino IDE, go to File->Preferences and under _Additional boards manager URLs_ paste this URL:

        https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

    If you have other URLs you can separate them using a comma.

2.  In the Arduino IDE, go to the library manager and install all the necessary libraries:
    - [WiFi101](https://docs.arduino.cc/libraries/wifi101/)
    - [Adafruit AHRS](https://github.com/adafruit/Adafruit_AHRS)
    - [Adafruit FXAS21002C](https://github.com/adafruit/Adafruit_FXAS21002C)
    - [Adafruit FXOS8700](https://github.com/adafruit/Adafruit_FXOS8700)
    - [OSC](https://github.com/CNMAT/OSC)
3.  Write the necessary information in the [secrets.h](./Arduino%20Sketches/ESP32C3_send_IMU_and_encoder_Data_OSC/secrets.h) file to allow the board to access WiFi. If the file doesn't exist create it in the sketch directory.
4.  Connect the board to the computer, compile and upload the code.

## How to run the Electron App 

### Production version (Windows)
From the [releases](https://github.com/ludovicodimartino/astrolabio-arduino/releases) download the latest Installer version.
Run the installer on your laptop and the program should start automatically.

### Dev version
You must have node.js installed in your system. You can download it from [here](https://nodejs.org/en).

Clone the repository.

Navigate to the [Digital Twin Desktop Application](./Digital%20Twin%20Desktop%20Application/) folder and run the following command:

    npm install

To start the application in development mode run:

    npm start


