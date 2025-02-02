#include <SPI.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <OSCBundle.h>

// Sensor libraries
#include <Adafruit_FXAS21002C.h>
#include <Adafruit_FXOS8700.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// #define DEBUG

#ifdef DEBUG
#define DEBUGPRINTLN Serial.println
#define DEBUGPRINT Serial.print
#else
#define DEBUGPRINTLN  // debug
#define DEBUGPRINT    // debug
#endif

#define BOARD_ID "1"  // An ID associated with the board to send data with ESP-NOW to the master.

uint8_t masterMACAddress[] = { 0x84, 0xCC, 0xA8, 0xA8, 0x58, 0xC5 };  // The MAC address of the master

Adafruit_FXOS8700 accelmag = Adafruit_FXOS8700(0x8700A, 0x8700B);  // assign a unique ID to the accelerometer and magnetometer for the I2C.
Adafruit_FXAS21002C gyro = Adafruit_FXAS21002C(0x0021002C);        // assign a unique ID to the gyroscope for the I2C.

sensors_event_t aevent, mevent, gevent;  // Sensor events.
OSCBundle bundle;                        // OSC bundle message.

const unsigned short sendFrequency = 10;  // number of message to send every second.

/* This function sends an OSC message to the master using the ESP-NOW protocol */
void sendToMaster() {
  DEBUGPRINTLN("Bundle size: %dbyte", sizeof(bundle));
  if (esp_now_send(masterMACAddress, (uint8_t *)&bundle, sizeof(bundle)) != 0) {
    DEBUGPRINTLN("Error sending the data");
  }
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
  Wire.begin(2, 0);

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

  DEBUGPRINTLN("Init ESP-NOW");
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    DEBUGPRINTLN("Error initializing ESP-NOW");
    ESP.deepSleep(0);
  }

  // Set ESP-NOW role
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  // Register peer
  if (esp_now_add_peer(masterMACAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0) != 0) {
    DEBUGPRINTLN("Failed to add peer");
    ESP.deepSleep(0);
  }
}

void loop() {
  // get new sensors event
  accelmag.getEvent(&aevent, &mevent);
  gyro.getEvent(&gevent);

  /*******************************
    Prepare an OSC bundle message 
  *******************************/

  // accelerometer data
  bundle.add(BOARD_ID "/acc/x").add(aevent.acceleration.x);
  bundle.add(BOARD_ID "/acc/y").add(aevent.acceleration.y);
  bundle.add(BOARD_ID "/acc/z").add(aevent.acceleration.z);

  // magnetometer data
  bundle.add(BOARD_ID "/mag/x").add(mevent.magnetic.x);
  bundle.add(BOARD_ID "/mag/y").add(mevent.magnetic.y);
  bundle.add(BOARD_ID "/mag/z").add(mevent.magnetic.z);

  // gyroscope data
  bundle.add(BOARD_ID "/gyro/x").add(gevent.gyro.x);
  bundle.add(BOARD_ID "/gyro/y").add(gevent.gyro.y);
  bundle.add(BOARD_ID "/gyro/z").add(gevent.gyro.z);

  // send the message
  sendToMaster();
  bundle.empty();
  delay(1000 / sendFrequency);  // send every 1000/sendFrequancy ms
}

#ifdef DEBUG
void printWifiData() {
  Serial.println("Board WiFi Information:");

  // print the MAC address of the board.
  Serial.print("ESP Board MAC Address: ");
  Serial.println(WiFi.macAddress());
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