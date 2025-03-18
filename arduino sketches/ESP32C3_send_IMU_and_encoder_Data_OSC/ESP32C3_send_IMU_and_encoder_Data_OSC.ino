/**
 * CODE FOR ESP32C3 superMini
 * N.B. REMEMBER TO CREATE THE secrets.h FILE THE WIFI CREDENTIALS AS FOLLOWS
 * #define SECRET_SSID "NetworkName"
 * #define SECRET_PASS "Password"
 *
 * @author Ludovico Di Martino
 */

// Communication libraries
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>

// IMU Sensor libraries
#include <Adafruit_FXAS21002C.h>
#include <Adafruit_FXOS8700.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_Sensor_Calibration.h>

// Library with the secrets (Wifi name and password)
#include "secrets.h"

// The DEBUG variable for debugging purposes (printing in the serial monitor)
#define DEBUG

#ifdef DEBUG
#define DEBUGPRINTLN Serial.println
#define DEBUGPRINT Serial.print
#else
#define DEBUGPRINTLN // debug
#define DEBUGPRINT   // debug
#endif

#define DT_A_PIN 4  // Alidada encoder pin 1
#define DT_B_PIN 2  // Rete encoder pin 1
#define CLK_A_PIN 3 // Alidada encoder pin 2
#define CLK_B_PIN 1 // Rete encoder pin 2
#define SW_PIN 0    // Encoder switch pin

#define SDA_PIN 5 // I2C SDA pin (used for the IMU sensor)
#define SCL_PIN 6 // I2C SCL pin (used for the IMU sensor)

#define WIFI_ST_LED 8 // The led that signals the wifi status

#define DEBOUNCE_TIME_SW 2000 // The interval used for switch debouncing

char ssid[] = SECRET_SSID; // network SSID (name)
char pass[] = SECRET_PASS; // network pass (use for WPA, or use as key for WEP)

WiFiUDP Udp;                                     // A UDP instance to send and receive packets over UDP
const IPAddress multicastIPAddr(239, 255, 0, 1); // remote IP address
const unsigned int remotePort = 9999;            // remote port to send OSC
const unsigned int localPort = 8888;             // local port to listen for OSC packets

Adafruit_FXOS8700 accelmag = Adafruit_FXOS8700(0x8700A, 0x8700B); // assign a unique ID to the accelerometer and magnetometer for the I2C
Adafruit_FXAS21002C gyro = Adafruit_FXAS21002C(0x0021002C);       // assign a unique ID to the gyroscope for the I2C

// Depending on where the calibration data is stored, the cal variable is different
#if defined(ADAFRUIT_SENSOR_CALIBRATION_USE_EEPROM)
  Adafruit_Sensor_Calibration_EEPROM cal;
#else
  Adafruit_Sensor_Calibration_SDFat cal;
#endif

volatile int32_t counterEncA = 0;                   // Counter for the alidada rotary encoder
volatile int32_t counterEncB = 0;                   // Counter for the rete rotary encoder
volatile bool newDataToReadEncA = false;            // New data to read for the encoder A
volatile bool newDataToReadEncB = false;            // New data to read for the encoder B
volatile unsigned long lastInterruptSwitchTime = 0; // The time when the last switch interrupt was executed

#ifdef DEBUG
void displaySensorDetails(void)
{
  Serial.print("Calibrations found on ");
  if(cal.hasEEPROM()) Serial.println("EEPROM: ");
  if(cal.hasFLASH()) Serial.println("FLASH: ");
  Serial.print("\tMagnetic Hard Offset: ");
  for (int i=0; i<3; i++) {
    Serial.print(cal.mag_hardiron[i]); 
    if (i != 2) Serial.print(", ");
  }
  Serial.println();
  
  Serial.print("\tMagnetic Soft Offset: ");
  for (int i=0; i<9; i++) {
    Serial.print(cal.mag_softiron[i]); 
    if (i != 8) Serial.print(", ");
  }
  Serial.println();

  Serial.print("\tMagnetic Field Magnitude: ");
  Serial.println(cal.mag_field);

  Serial.print("\tGyro Zero Rate Offset: ");
  for (int i=0; i<3; i++) {
    Serial.print(cal.gyro_zerorate[i]); 
    if (i != 2) Serial.print(", ");
  }
  Serial.println();

  Serial.print("\tAccel Zero G Offset: ");
  for (int i=0; i<3; i++) {
    Serial.print(cal.accel_zerog[i]); 
    if (i != 2) Serial.print(", ");
  }
  Serial.println();

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

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  // Signaling the connection with the led
  digitalWrite(WIFI_ST_LED, LOW);

  DEBUGPRINTLN("Connection succeeded!");
  DEBUGPRINTLN("----------------------------------------");
#ifdef DEBUG
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
  DEBUGPRINT(multicastIPAddr);
  DEBUGPRINT(":");
  DEBUGPRINTLN(remotePort);
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  // Signaling the disconnection with the led
  digitalWrite(WIFI_ST_LED, HIGH);

  DEBUGPRINT("WiFi lost connection. Reason: ");
  DEBUGPRINTLN(info.wifi_sta_disconnected.reason);
  DEBUGPRINTLN("Trying to Reconnect");
}

void rotary_dt_A_down()
{
  // encoder in 00 state, pin 1 jumped down last
  if (digitalRead(CLK_A_PIN) == 0)
  {
    counterEncA--;
    newDataToReadEncA = true;
    attachInterrupt(digitalPinToInterrupt(DT_A_PIN), rotary_dt_A_up, RISING);
    attachInterrupt(digitalPinToInterrupt(CLK_A_PIN), rotary_clk_A_up, RISING);
  }
  // encoder in 01 state, waiting for pin 2 to jump down
  else
  {
    detachInterrupt(digitalPinToInterrupt(DT_A_PIN));
    attachInterrupt(digitalPinToInterrupt(CLK_A_PIN), rotary_clk_A_down, FALLING);
  }
}

void rotary_clk_A_down()
{
  // encoder in 00 state, pin 2 jumped down last
  if (digitalRead(DT_A_PIN) == 0)
  {
    counterEncA++;
    newDataToReadEncA = true;
    attachInterrupt(digitalPinToInterrupt(DT_A_PIN), rotary_dt_A_up, RISING);
    attachInterrupt(digitalPinToInterrupt(CLK_A_PIN), rotary_clk_A_up, RISING);
  }
  // encoder in 10 state, waiting for pin 1 to jump down
  else
  {
    attachInterrupt(digitalPinToInterrupt(DT_A_PIN), rotary_dt_A_down, FALLING);
    detachInterrupt(digitalPinToInterrupt(CLK_A_PIN));
  }
}

void rotary_dt_A_up()
{
  // encoder in 11 state, pin 1 jumped up last
  if (digitalRead(CLK_A_PIN) == 1)
  {
    counterEncA--;
    newDataToReadEncA = true;
    attachInterrupt(digitalPinToInterrupt(DT_A_PIN), rotary_dt_A_down, FALLING);
    attachInterrupt(digitalPinToInterrupt(CLK_A_PIN), rotary_clk_A_down, FALLING);
  }
  // encoder in 10 state, waiting for pin 2 to jump up
  else
  {
    detachInterrupt(digitalPinToInterrupt(DT_A_PIN));
    attachInterrupt(digitalPinToInterrupt(CLK_A_PIN), rotary_clk_A_up, RISING);
  }
}

void rotary_clk_A_up()
{
  // encoder in 11 state, pin 2 jumped up last
  if (digitalRead(DT_A_PIN) == HIGH)
  {
    counterEncA++;
    newDataToReadEncA = true;
    attachInterrupt(digitalPinToInterrupt(DT_A_PIN), rotary_dt_A_down, FALLING);
    attachInterrupt(digitalPinToInterrupt(CLK_A_PIN), rotary_clk_A_down, FALLING);
  }
  // encoder in 01 state, waiting for pin 1 to jump up
  else
  {
    attachInterrupt(digitalPinToInterrupt(DT_A_PIN), rotary_dt_A_up, RISING);
    detachInterrupt(digitalPinToInterrupt(CLK_A_PIN));
  }
}

void rotary_dt_B_down()
{
  // encoder in 00 state, pin 1 jumped down last
  if (digitalRead(CLK_B_PIN) == 0)
  {
    counterEncB--;
    newDataToReadEncB = true;
    attachInterrupt(digitalPinToInterrupt(DT_B_PIN), rotary_dt_B_up, RISING);
    attachInterrupt(digitalPinToInterrupt(CLK_B_PIN), rotary_clk_B_up, RISING);
  }
  // encoder in 01 state, waiting for pin 2 to jump down
  else
  {
    detachInterrupt(digitalPinToInterrupt(DT_B_PIN));
    attachInterrupt(digitalPinToInterrupt(CLK_B_PIN), rotary_clk_B_down, FALLING);
  }
}

void rotary_clk_B_down()
{
  // encoder in 00 state, pin 2 jumped down last
  if (digitalRead(DT_B_PIN) == 0)
  {
    counterEncB++;
    newDataToReadEncB = true;
    attachInterrupt(digitalPinToInterrupt(DT_B_PIN), rotary_dt_B_up, RISING);
    attachInterrupt(digitalPinToInterrupt(CLK_B_PIN), rotary_clk_B_up, RISING);
  }
  // encoder in 10 state, waiting for pin 1 to jump down
  else
  {
    attachInterrupt(digitalPinToInterrupt(DT_B_PIN), rotary_dt_B_down, FALLING);
    detachInterrupt(digitalPinToInterrupt(CLK_B_PIN));
  }
}

void rotary_dt_B_up()
{
  // encoder in 11 state, pin 1 jumped up last
  if (digitalRead(CLK_B_PIN) == 1)
  {
    counterEncB--;
    newDataToReadEncB = true;
    attachInterrupt(digitalPinToInterrupt(DT_B_PIN), rotary_dt_B_down, FALLING);
    attachInterrupt(digitalPinToInterrupt(CLK_B_PIN), rotary_clk_B_down, FALLING);
  }
  // encoder in 10 state, waiting for pin 2 to jump up
  else
  {
    detachInterrupt(digitalPinToInterrupt(DT_B_PIN));
    attachInterrupt(digitalPinToInterrupt(CLK_B_PIN), rotary_clk_B_up, RISING);
  }
}

void rotary_clk_B_up()
{
  // encoder in 11 state, pin 2 jumped up last
  if (digitalRead(DT_B_PIN) == HIGH)
  {
    counterEncB++;
    newDataToReadEncB = true;
    attachInterrupt(digitalPinToInterrupt(DT_B_PIN), rotary_dt_B_down, FALLING);
    attachInterrupt(digitalPinToInterrupt(CLK_B_PIN), rotary_clk_B_down, FALLING);
  }
  // encoder in 01 state, waiting for pin 1 to jump up
  else
  {
    attachInterrupt(digitalPinToInterrupt(DT_B_PIN), rotary_dt_B_up, RISING);
    detachInterrupt(digitalPinToInterrupt(CLK_B_PIN));
  }
}

void setup()
{
#ifdef DEBUG
  // Serial monitor setup
  Serial.begin(115200);
  while(!Serial){
    delay(500);
  }
#endif

  // I2C communication pins: set SDA pin and SCL pin
  Wire.begin(SDA_PIN, SCL_PIN);

  // load the calibration for the IMU
  bool calBegin = cal.begin();
  bool loadCal = cal.loadCalibration();
  if (!calBegin) {
    DEBUGPRINTLN("Failed to initialize calibration helper");
  } else if (!loadCal) {
    DEBUGPRINTLN("No calibration loaded/found");
  }

  // accelerometer and magnetometer initialization
  if (!accelmag.begin())
  {
    // There was a problem detecting the FXOS8700
    DEBUGPRINTLN("No FXOS8700 detected ... Check your wiring!");
    ESP.deepSleep(0);
  }

  // gyroscope initialization
  if (!gyro.begin())
  {
    // there was a problem detecting the FXAS21002C
    DEBUGPRINTLN("No FXAS21002C detected ... Check your wiring!");
    ESP.deepSleep(0);
  }

#ifdef DEBUG
  // print sensor data
  displaySensorDetails();
#endif

  // Initialize the Wifi LED
  pinMode(WIFI_ST_LED, OUTPUT);
  
  // Turn off the wifi status led (on by default)
  digitalWrite(WIFI_ST_LED, HIGH);

  // Initialize the pins for the rotary encoders with the internal pullup
  pinMode(DT_A_PIN, INPUT_PULLUP);
  pinMode(DT_B_PIN, INPUT_PULLUP);
  pinMode(CLK_A_PIN, INPUT_PULLUP);
  pinMode(CLK_B_PIN, INPUT_PULLUP);
  pinMode(SW_PIN, INPUT_PULLUP);

  // Read the initial encoder states
  int8_t dt_A = digitalRead(DT_A_PIN);
  int8_t clk_A = digitalRead(CLK_A_PIN);
  int8_t dt_B = digitalRead(DT_B_PIN);
  int8_t clk_B = digitalRead(CLK_B_PIN);

  // Initial position of the encoder A 11
  if ((dt_A == HIGH) && (clk_A == HIGH))
  {
    attachInterrupt(digitalPinToInterrupt(DT_A_PIN), rotary_dt_A_down, FALLING);
    attachInterrupt(digitalPinToInterrupt(CLK_A_PIN), rotary_clk_A_down, FALLING);
  }

  // Initial position of the encoder A 00
  if ((dt_A == LOW) && (clk_A == LOW))
  {
    attachInterrupt(digitalPinToInterrupt(DT_A_PIN), rotary_dt_A_down, RISING);
    attachInterrupt(digitalPinToInterrupt(CLK_A_PIN), rotary_clk_A_up, RISING);
  }

  // Initial position of the encoder B 11
  if ((dt_B == HIGH) && (clk_B == HIGH))
  {
    DEBUGPRINTLN("Encoder B position 11");
    attachInterrupt(digitalPinToInterrupt(DT_B_PIN), rotary_dt_B_down, FALLING);
    attachInterrupt(digitalPinToInterrupt(CLK_B_PIN), rotary_clk_B_down, FALLING);
  }

  // Initial position of the encoder B 00
  if ((dt_B == LOW) && (clk_B == LOW))
  {
    DEBUGPRINTLN("Encoder B position 00");
    attachInterrupt(digitalPinToInterrupt(DT_B_PIN), rotary_dt_B_down, RISING);
    attachInterrupt(digitalPinToInterrupt(CLK_B_PIN), rotary_clk_B_up, RISING);
  }

  // network connection
  DEBUGPRINT("Attempting to connect to network: ");
  DEBUGPRINTLN(ssid);

  // Set the Wifi mode
  WiFi.mode(WIFI_STA);

  // Delete the old configurations
  WiFi.disconnect(true);

  delay(1000);

  WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  WiFi.begin(ssid, pass);

  // Wait for the connection to be ready
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
  }

  // Send initial reset message to the client
  OSCMessage msg("/reset");
  Udp.beginPacket(multicastIPAddr, remotePort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

void loop()
{
  // Reading the data from the rotary encoders
  if (newDataToReadEncA)
  {
    OSCMessage msg("/encA");
    msg.add(counterEncA);
    Udp.beginPacket(multicastIPAddr, remotePort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
    DEBUGPRINTLN(counterEncA);
    newDataToReadEncA = false;
  }
  if (newDataToReadEncB)
  {
    OSCMessage msg("/encB");
    msg.add(counterEncB);
    Udp.beginPacket(multicastIPAddr, remotePort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
    DEBUGPRINTLN(counterEncB);
    newDataToReadEncB = false;
  }

  // Check if the switch was pressed
  unsigned long currentTime = millis();
  // Debounce logic (Ignore interrupts within DEBOUNCE_TIME_SW)
  if (currentTime - lastInterruptSwitchTime > DEBOUNCE_TIME_SW && digitalRead(SW_PIN) == LOW)
  {
    // reset the counters
    counterEncA = 0;
    counterEncB = 0;
    DEBUGPRINTLN("ENCODER SWITCH PRESSED");

    // send reset message
    OSCMessage msg("/reset");
    Udp.beginPacket(multicastIPAddr, remotePort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
    lastInterruptSwitchTime = currentTime;
  }

  sensors_event_t aevent, mevent, gevent;

  // get new sensors event
  accelmag.getEvent(&aevent, &mevent);
  gyro.getEvent(&gevent);

  // calibration step
  cal.calibrate(mevent);
  cal.calibrate(aevent);
  cal.calibrate(gevent);

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
  Udp.beginPacket(multicastIPAddr, remotePort);
  bundle.send(Udp);
  Udp.endPacket();
  bundle.empty();
}
