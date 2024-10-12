#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <OSCBundle.h>

// Sensor libraries
#include <Adafruit_FXAS21002C.h>
#include <Adafruit_FXOS8700.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Libraries with the secrets
#include "arduino_secrets.h"


Adafruit_FXOS8700 accelmag = Adafruit_FXOS8700(0x8700A, 0x8700B);       // assign a unique ID to the accelerometer and magnetometer for the I2C
Adafruit_FXAS21002C gyro = Adafruit_FXAS21002C(0x0021002C);             // assign a unique ID to the gyroscope for the I2C

char ssid[] = SECRET_SSID;       // network SSID (name)
char pass[] = SECRET_PASS;       // network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status

WiFiUDP Udp;                                // A UDP instance to send and receive packets over UDP
const IPAddress remoteIP(192,168,43,238);   // remote IP of the receiving computer
const unsigned int remotePort = 9999;       // remote port to send OSC
const unsigned int localPort = 8888;        // local port to listen for OSC packets

const unsigned short sendFrequency = 10;     // number of message to send every second


void setup() {
  //Serial monitor setup
  Serial.begin(9600);

  // wait for the Serial Monitor
  while (!Serial) {
    delay(1);
  }

  // accelerometer and magnetometer initialization
  if (!accelmag.begin()) {
    // There was a problem detecting the FXOS8700
    Serial.println("No FXOS8700 detected ... Check your wiring!");
    while (1);
  }

  // gyroscope initialization
  if (!gyro.begin()) {
    // there was a problem detecting the FXAS21002C
    Serial.println("No FXAS21002C detected ... Check your wiring!");
    while (1);
  }

  // print sensor data
  displaySensorDetails();

  // network connection
  Serial.print("Attempting to connect to network: ");
  Serial.println(ssid);
  
  // Connect to WPA/WPA2 network
  status = WiFi.begin(ssid, pass);
  Serial.println("Connecting...");

  // attempt to connect to Wifi network
  while (status != WL_CONNECTED) {
    Serial.println(".");
    delay(100);
  }

  // Connection succeeded
  Serial.println("Connection succeeded!");
  Serial.println("----------------------------------------");
  printWifiData();
  Serial.println("----------------------------------------");

  // Initializing the UDP
  Serial.println("Starting UDP...");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(localPort);
  Serial.println("----------------------------------------");
  Serial.println("");
  Serial.print("Sending data to: ");
  Serial.print(remoteIP);
  Serial.print(":");
  Serial.println(remotePort);
}

void loop() {
  sensors_event_t aevent, mevent, gevent;

  // get new sensors event
  accelmag.getEvent(&aevent, &mevent);
  gyro.getEvent(&gevent);

  // Prepare an OSC bundle message
  OSCBundle bundle;

  // accelerometer data
  bundle.add("/acc/x").add(aevent.acceleration.x);
  bundle.add("/acc/y").add(aevent.acceleration.y);
  bundle.add("/acc/z").add(aevent.acceleration.z);

  // magnetometer data
  bundle.add("/mag/x").add(mevent.magnetic.x);
  bundle.add("/mag/y").add(mevent.magnetic.y);
  bundle.add("/mag/z").add(mevent.magnetic.z);

  // gyroscope data
  bundle.add("/gyro/x").add(gevent.gyro.x);
  bundle.add("/gyro/y").add(gevent.gyro.y);
  bundle.add("/gyro/z").add(gevent.gyro.z);

  // send the message
  Udp.beginPacket(remoteIP, remotePort);
  bundle.send(Udp);
  Udp.endPacket();
  bundle.empty();
  delay(1000/sendFrequency); // send every 1000/sendFrequancy ms
}

void printWifiData() {
  Serial.println("Board WiFi Information:");
  // print the board's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.println();
  Serial.println("Network Information:");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI): ");
  Serial.println(rssi);

  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type: ");
  Serial.println(encryption, HEX);
  Serial.println();
}

void displaySensorDetails(void) {
  sensor_t accel, mag, gyroSensor;
  accelmag.getSensor(&accel, &mag);
  Serial.println("------------------------------------");
  Serial.println("ACCELEROMETER");
  Serial.println("------------------------------------");
  Serial.print("Sensor:       ");
  Serial.println(accel.name);
  Serial.print("Driver Ver:   ");
  Serial.println(accel.version);
  Serial.print("Unique ID:    0x");
  Serial.println(accel.sensor_id, HEX);
  Serial.print("Min Delay:    ");
  Serial.print(accel.min_delay);
  Serial.println(" s");
  Serial.print("Max Value:    ");
  Serial.print(accel.max_value, 4);
  Serial.println(" m/s^2");
  Serial.print("Min Value:    ");
  Serial.print(accel.min_value, 4);
  Serial.println(" m/s^2");
  Serial.print("Resolution:   ");
  Serial.print(accel.resolution, 8);
  Serial.println(" m/s^2");
  Serial.println("------------------------------------");
  Serial.println("");
  Serial.println("------------------------------------");
  Serial.println("MAGNETOMETER");
  Serial.println("------------------------------------");
  Serial.print("Sensor:       ");
  Serial.println(mag.name);
  Serial.print("Driver Ver:   ");
  Serial.println(mag.version);
  Serial.print("Unique ID:    0x");
  Serial.println(mag.sensor_id, HEX);
  Serial.print("Min Delay:    ");
  Serial.print(accel.min_delay);
  Serial.println(" s");
  Serial.print("Max Value:    ");
  Serial.print(mag.max_value);
  Serial.println(" uT");
  Serial.print("Min Value:    ");
  Serial.print(mag.min_value);
  Serial.println(" uT");
  Serial.print("Resolution:   ");
  Serial.print(mag.resolution);
  Serial.println(" uT");
  Serial.println("------------------------------------");
  Serial.println("");
  gyro.getSensor(&gyroSensor);
  Serial.println("------------------------------------");
  Serial.println("GYROSCOPE");
  Serial.println("------------------------------------");
  Serial.print("Sensor:       ");
  Serial.println(gyroSensor.name);
  Serial.print("Driver Ver:   ");
  Serial.println(gyroSensor.version);
  Serial.print("Unique ID:    0x");
  Serial.println(gyroSensor.sensor_id, HEX);
  Serial.print("Max Value:    ");
  Serial.print(gyroSensor.max_value);
  Serial.println(" rad/s");
  Serial.print("Min Value:    ");
  Serial.print(gyroSensor.min_value);
  Serial.println(" rad/s");
  Serial.print("Resolution:   ");
  Serial.print(gyroSensor.resolution);
  Serial.println(" rad/s");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}