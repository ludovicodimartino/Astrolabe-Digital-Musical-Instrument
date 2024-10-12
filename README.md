# Atrolabio project
Research training project for the Master's Degree Program in Computer Engineering. The topic of the activity is the design and development of the digital musical instrument based on the astrolabe from the [_Giovanni Poleni_ museum](https://www.musei.unipd.it/it/fisica#) in Padua for the show _Caravanserraglio_.

## Components
For the prototype, the following components were used: 

- [Arduino MKR 1000 WiFi](https://docs.arduino.cc/hardware/mkr-1000-wifi/) 
- [Adafruit Precision NXP 9-DOF Breakout Board - FXOS8700 + FXAS21002](https://www.adafruit.com/product/3463)

For the communication between the microcontroller and the sensor, the I<sup>2</sup>C serial communication system was used. Here’s how everything was wired:
![image](https://cdn-learn.adafruit.com/assets/assets/000/040/748/large1024/sensors_NXP9DOFBREADBOARD.png?1491841114)

## How to run the code
1. In the Arduino IDE, go to the board manager and install the package for the board MKR1000.
2. In the Arduino IDE, go to the library manager and install all the necessary libraries:
    - [WiFi101](https://docs.arduino.cc/libraries/wifi101/)
    - [Adafruit AHRS](https://github.com/adafruit/Adafruit_AHRS)
    - [Adafruit FXAS21002C](https://github.com/adafruit/Adafruit_FXAS21002C)
    - [Adafruit FXOS8700](https://github.com/adafruit/Adafruit_FXOS8700)
    - [OSC](https://github.com/CNMAT/OSC)
3. Write the necessary information in the [arduino_secrets.h](/arduino%20sketches/send_sensor_data_OSC/arduino_secrets.h) file to allow Arduino to access WiFi.
4. Set the IP of your computer in the `remoteIP` variable.
5. Connect the board to the computer, compile and upload the code.
