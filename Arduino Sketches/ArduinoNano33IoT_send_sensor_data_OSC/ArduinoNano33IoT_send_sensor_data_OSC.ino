#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <OSCBundle.h>

// Sensor libraries
#include <Adafruit_FXAS21002C.h>
#include <Adafruit_FXOS8700.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Libraries with the secrets
#include "arduino_secrets.h"

// #define DEBUG

#ifdef DEBUG 
#define DEBUGPRINTLN Serial.println
#define DEBUGPRINT Serial.print
#else 
#define DEBUGPRINTLN // debug
#define DEBUGPRINT // debug
#endif


Adafruit_FXOS8700 accelmag = Adafruit_FXOS8700(0x8700A, 0x8700B);       // assign a unique ID to the accelerometer and magnetometer for the I2C.
Adafruit_FXAS21002C gyro = Adafruit_FXAS21002C(0x0021002C);             // assign a unique ID to the gyroscope for the I2C.

char ssid[] = SECRET_SSID;       // network SSID (name).
char pass[] = SECRET_PASS;       // network password (use for WPA, or use as key for WEP).
int status = WL_IDLE_STATUS;     // the Wifi radio's status.

WiFiUDP Udp;                                 // A UDP instance to send and receive packets over UDP.
const IPAddress multicastAddress(225,0,0,1); // multicast address.
const unsigned int remotePort = 9999;        // remote port to send OSC.
const unsigned int localPort = 8888;         // local port to listen for OSC packets.

const unsigned short sendFrequency = 50;     // number of message to send every second.

unsigned long previousMillis = 0;            // last time the connection status was checked.
unsigned long interval = 5000;               // the interval between two connection checks.
unsigned short connectionLostFlag = 0;       // flag that signals if in the previous loop the cannection was lost.
unsigned int retrialCount = 0;               // number of time the Arduino tried to connect to the wifi.
pin_size_t wifiLed = LED_BUILTIN;            // the led used to signal the WiFi status.
PinStatus wifiLedStatus = LOW;               // the WiFi led status.

/**
* This function performs the operations that should be done after the connection is acquired.
*/
void connectionSuccessful(){
  // Signaling the connection with the led
  wifiLedStatus = HIGH;
  retrialCount = 0;
  digitalWrite(wifiLed, HIGH);

  // Reset the connection lost flag
  connectionLostFlag = 0;

  // Connection succeeded
  DEBUGPRINTLN("Connection succeeded!");
  DEBUGPRINTLN("----------------------------------------");
#ifdef DEBUG
  printWifiData();
#endif
  DEBUGPRINTLN("----------------------------------------");

  // Initializing the UDP
  DEBUGPRINTLN("Starting UDP...");
  Udp.begin(localPort);
  DEBUGPRINT("Local port: ");
  DEBUGPRINTLN(localPort);
  DEBUGPRINTLN("----------------------------------------");
  DEBUGPRINTLN("");
  DEBUGPRINT("Sending data to: ");
  DEBUGPRINT(multicastAddress);
  DEBUGPRINT(":");
  DEBUGPRINTLN(remotePort);
}

void setup() {

#ifdef DEBUG
  //Serial monitor setup
  Serial.begin(9600);

  // wait for the Serial Monitor
  while (!Serial) {
    delay(1);
  }
#endif

  // Wire.begin(A4,A5);

  // accelerometer and magnetometer initialization
  if (!accelmag.begin()) {
    // There was a problem detecting the FXOS8700
    DEBUGPRINTLN("No FXOS8700 detected ... Check your wiring!");
    while (1);
  }

  // gyroscope initialization
  if (!gyro.begin()) {
    // there was a problem detecting the FXAS21002C
    DEBUGPRINTLN("No FXAS21002C detected ... Check your wiring!");
    while (1);
  }

  // Wifi-led init
  pinMode(wifiLed, OUTPUT);

#ifdef DEBUG
  // print sensor data
  displaySensorDetails();
#endif

  // network connection
  DEBUGPRINT("Attempting to connect to network: ");
  DEBUGPRINTLN(ssid);
  
  // Connect to WPA/WPA2 network
  DEBUGPRINTLN("Connecting...");
  
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    if(++retrialCount%5){
      wifiLedStatus = (wifiLedStatus == HIGH) ? LOW : HIGH;
      digitalWrite(wifiLed, wifiLedStatus);
    } 
    DEBUGPRINTLN(".");
    delay(100);
  }

  connectionSuccessful();
  
}

void loop() {

  // Manage connection lost
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if(WiFi.status() != WL_CONNECTED){
      connectionLostFlag = 1; // Signal that the connection was lost
      WiFi.begin(ssid, pass);
      if(++retrialCount%5){
        DEBUGPRINTLN("Connection lost. Reconnecting...");
        wifiLedStatus = (wifiLedStatus == HIGH) ? LOW : HIGH;
        digitalWrite(wifiLed, wifiLedStatus);
      } 
      interval = 100; // Retry often, until you are connected again
      return; // Don't go on if the connection is lost
    } else {
      if(connectionLostFlag){
        DEBUGPRINTLN("Back online");
        connectionSuccessful();
      }
      interval = 5000;
    }
  }
  
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
  Udp.beginPacket(multicastAddress, remotePort);
  bundle.send(Udp);
  Udp.endPacket();
  bundle.empty();
  delay(1000/sendFrequency); // send every 1000/sendFrequancy ms
}

#ifdef DEBUG
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
#endif