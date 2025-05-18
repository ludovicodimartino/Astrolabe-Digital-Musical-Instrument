/**
* OSC Benchmark
*/
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCBundle.h>

// Sensor libraries
#include <Adafruit_FXAS21002C.h>
#include <Adafruit_FXOS8700.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Libraries with the secrets
#include "secrets.h"

#define DEBUG

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

WiFiUDP Udp;                                 // A UDP instance to send and receive packets over UDP.
const IPAddress multicastAddress(239, 255, 0, 1); // multicast address.
const unsigned int remotePort = 9999;        // remote port to send OSC.
const unsigned int localPort = 8888;         // local port to listen for OSC packets.

const unsigned short sendFrequency = 10;     // number of message to send every second.

#ifdef DEBUG 
unsigned long previousMillis = 0;            // last time the WifiCheck was done.
#endif

/**
* This function performs the operations that should be done after the connection is acquired.
*/
void connectionSuccessful(){
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

  // I2C communication pins: set GPIO2 for SDA and GPIO0 for SCL
  Wire.begin(5,6);

  // accelerometer and magnetometer initialization
  if (!accelmag.begin()) {
    // There was a problem detecting the FXOS8700
    DEBUGPRINTLN("No FXOS8700 detected ... Check your wiring!");
    ESP.deepSleep(0);
  }

  // gyroscope initialization
  if (!gyro.begin()) {
    // there was a problem detecting the FXAS21002C
    DEBUGPRINTLN("No FXAS21002C detected ... Check your wiring!");
    ESP.deepSleep(0);
  }

#ifdef DEBUG
  // print sensor data
  displaySensorDetails();
#endif

  // network connection
  DEBUGPRINT("Attempting to connect to network: ");
  DEBUGPRINTLN(ssid);
  
  // Connect to WPA/WPA2 network
  DEBUGPRINTLN("Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  while (WiFi.status() != WL_CONNECTED) {
    DEBUGPRINTLN(".");
    delay(1000);
  }
  connectionSuccessful();
}

void loop() {
#ifdef DEBUG
  // Debug for the WiFi connection
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 30000){
    switch (WiFi.status()){
        case WL_NO_SSID_AVAIL:
          DEBUGPRINTLN("Configured SSID cannot be reached");
          break;
        case WL_CONNECTED:
          DEBUGPRINTLN("Connection successfully established");
          break;
        case WL_CONNECT_FAILED:
          DEBUGPRINTLN("Connection failed");
          break;
    }
    previousMillis = currentMillis;
  }
#endif
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
