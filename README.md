# Astrolabio project

Research training project for the Master's Degree Program in Computer Engineering. The topic of the activity is the design and development of the digital musical instrument based on the astrolabe from the [_Giovanni Poleni_ museum](https://www.musei.unipd.it/it/fisica#) in Padua for the show _Caravanserraglio_.

## Project Structure

```
Astrolabio project
│
└───Arduino sketches    # the developed sketches for the Arduino board
│
└───puredata patches    # the developed Pure Data patches
│   │
│   └───abstractions    # reusable Pure Data Objects for reading the sensor data from the network
│   │
│   └───tests           # patches for testing the communication with the Arduino
│
└───Digital Twin Desktop Application       # Electron app for viewing the 3D astrolabe model
│
└─── README.md
```

## Hardware Components

For the prototype, the following components were used:

-   [ESP32-C3 Super Mini Dev Board](https://www.espboards.dev/esp32/esp32-c3-super-mini/)
-   [Adafruit Precision NXP 9-DOF Breakout Board - FXOS8700 + FXAS21002](https://www.adafruit.com/product/3463)
-   [Dual rotary encoder](https://docs.rs-online.com/f0a8/A700000009887456.pdf)
-   [LiPo Rider Plus](https://wiki.seeedstudio.com/Lipo-Rider-Plus/)
-   LiPo Battery 3.7V 1800mAh

For the communication between the microcontroller and the IMU (Adafruit Precision NXP 9-DOF), the I<sup>2</sup>C serial communication system was used. Here’s how everything was wired:
![image](./fritzing%20breadboard%20diagrams/ESP32C3Breadboard.png)

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
3.  Write the necessary information in the [secrets.h](/arduino%20sketches/ESP32C3_send_IMU_and_encoder_Data_OSC/secrets.h) file to allow the board to access WiFi.
4.  Connect the board to the computer, compile and upload the code.

## How to run the Electron App (Windows)

From the [releases](https://github.com/ludovicodimartino/astrolabio-arduino/releases) download the latest Installer version.
Run the installer on your laptop and the program should start automatically.
