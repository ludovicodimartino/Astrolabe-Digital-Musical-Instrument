/**
* CODE FOR ESP32
* N.B. REMEMBER TO WRITE IN THE secret.h FILE THE WIFI CREDENTIALS AS FOLLOWS:
* #define SECRET_SSID "NetworkName"
* #define SECRET_PASS "Password"
*
* Change the remote IP address to the IP of the receiving machine.
* You can use this sketch paird with the test_encoder.py program that display the counter.
*/
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

// Library with the secrets
#include "secrets.h"

// #define DEBUG

#ifdef DEBUG
#define DEBUGPRINTLN Serial.println
#define DEBUGPRINT Serial.print
#else
#define DEBUGPRINTLN  // debug
#define DEBUGPRINT    // debug
#endif

#define DT_A_PIN 4
#define DT_B_PIN 16
#define CLK_A_PIN 5
#define CLK_B_PIN 17
#define SW_PIN 18
#define WIFI_LED_PIN 2  // pin for the led signaling the status of the wifi

int numA = 0;
int numB = 0;

char ssid[] = SECRET_SSID;    // network SSID (name).
char pass[] = SECRET_PASS;    // network pass (use for WPA, or use as key for WEP).

WiFiUDP Udp;                                  // A UDP instance to send and receive packets over UDP.
const IPAddress remoteIP(192, 168, 43, 238);  // remote IP address.
const unsigned int remotePort = 9999;         // remote port to send OSC.
const unsigned int localPort = 8888;          // local port to listen for OSC packets.

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

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  // Signaling the connection with the led
  digitalWrite(WIFI_LED_PIN, HIGH);

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
  DEBUGPRINT(remoteIP);
  DEBUGPRINT(":");
  DEBUGPRINTLN(remotePort);
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  // Signaling with the led
  digitalWrite(WIFI_LED_PIN, LOW);
  DEBUGPRINTLN("Disconnected from WiFi access point");
  DEBUGPRINTLN("WiFi lost connection. Reason: ");
  DEBUGPRINTLN(info.wifi_sta_disconnected.reason);
  DEBUGPRINTLN("Trying to Reconnect");
  WiFi.begin(ssid, pass);
}

void rotary_dt_A_down() {
  DEBUGPRINTLN("ROTARY DT A DOWN");
  /* encoder in 00 state, pin A jumped down last */
  if (digitalRead(CLK_A_PIN) == 0) {
    /* for neutral state 00, this is the end of the cycle, but register the counterclockwise rotation only if pin A jumped last in both half cycles */
    // if (!neutral and lasta)
    // {
    numA = numA - 1;
    OSCMessage msg("/encA");
    msg.add((int32_t)numA);
    Udp.beginPacket(remoteIP, remotePort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
    DEBUGPRINTLN(numA);
    // }
    // lasta = true;
    attachInterrupt(digitalPinToInterrupt(DT_A_PIN), rotary_dt_A_up, RISING);
    attachInterrupt(digitalPinToInterrupt(CLK_A_PIN), rotary_clk_A_up, RISING);
  }
  /* encoder in 01 state, waiting for B to jump down */
  else {
    detachInterrupt(digitalPinToInterrupt(DT_A_PIN));
    attachInterrupt(digitalPinToInterrupt(CLK_A_PIN), rotary_clk_A_down, FALLING);
  }
}

void rotary_clk_A_down() {
  DEBUGPRINTLN("ROTARY CLK A DOWN");
  /* encoder in 00 state, pin B jumped down last */
  if (digitalRead(DT_A_PIN) == 0) {
    /* for neutral state 00, this is the end of the cycle, but register the clockwise rotation only if pin B jumped last in both half cycles */
    // if (!neutral and !lasta)
    // {
    numA = numA + 1;
    OSCMessage msg("/encA");
    msg.add((int32_t)numA);
    Udp.beginPacket(remoteIP, remotePort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
    DEBUGPRINTLN(numA);
    // }
    // lasta = false;
    attachInterrupt(digitalPinToInterrupt(DT_A_PIN), rotary_dt_A_up, RISING);
    attachInterrupt(digitalPinToInterrupt(CLK_A_PIN), rotary_clk_A_up, RISING);
  }
  /* encoder in 10 state, waiting for A to jump down */
  else {
    attachInterrupt(digitalPinToInterrupt(DT_A_PIN), rotary_dt_A_down, FALLING);
    detachInterrupt(digitalPinToInterrupt(CLK_A_PIN));
  }
}

void rotary_dt_A_up() {
  DEBUGPRINTLN("ROTARY DT A UP");
  /* encoder in 11 state, in A jumped up last */
  if (digitalRead(CLK_A_PIN) == 1) {
    /* for neutral state 11, this is the end of the cycle, but register the counterclockwise rotation only if pin A jumped last in both half cycles */
    // if (neutral and lasta)
    // {
    numA = numA - 1;
    OSCMessage msg("/encA");
    msg.add((int32_t)numA);
    Udp.beginPacket(remoteIP, remotePort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
    DEBUGPRINTLN(numA);
    // }
    // lasta = true;
    attachInterrupt(digitalPinToInterrupt(DT_A_PIN), rotary_dt_A_down, FALLING);
    attachInterrupt(digitalPinToInterrupt(CLK_A_PIN), rotary_clk_A_down, FALLING);
  }
  /* encoder in 10 state, waiting for B to jump up */
  else {
    detachInterrupt(digitalPinToInterrupt(DT_A_PIN));
    attachInterrupt(digitalPinToInterrupt(CLK_A_PIN), rotary_clk_A_up, RISING);
  }
}

void rotary_clk_A_up() {
  DEBUGPRINTLN("ROTARY CLK A UP");
  /* encoder in 11 state, pin B jumped up last */
  if (digitalRead(DT_A_PIN) == HIGH) {
    /* for neutral state 11, this is the end of the cycle, but register the clockwise rotation only if pin B jumped last in both half cycles */
    // if (neutral and !lasta)
    // {
    numA = numA + 1;
    OSCMessage msg("/encA");
    msg.add((int32_t)numA);
    Udp.beginPacket(remoteIP, remotePort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
    DEBUGPRINTLN(numA);
    // }
    // lasta = false;
    attachInterrupt(digitalPinToInterrupt(DT_A_PIN), rotary_dt_A_down, FALLING);
    attachInterrupt(digitalPinToInterrupt(CLK_A_PIN), rotary_clk_A_down, FALLING);
  }
  /* encoder in 01 state, waiting for A to jump up */
  else {
    attachInterrupt(digitalPinToInterrupt(DT_A_PIN), rotary_dt_A_up, RISING);
    detachInterrupt(digitalPinToInterrupt(CLK_A_PIN));
  }
}

void rotary_dt_B_down() {
  DEBUGPRINTLN("ROTARY DT B DOWN");
  /* encoder in 00 state, pin A jumped down last */
  if (digitalRead(CLK_B_PIN) == 0) {
    /* for neutral state 00, this is the end of the cycle, but register the counterclockwise rotation only if pin A jumped last in both half cycles */
    // if (!neutral and lasta)
    // {
    numB = numB - 1;
    OSCMessage msg("/encB");
    msg.add((int32_t)numB);
    Udp.beginPacket(remoteIP, remotePort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
    DEBUGPRINTLN(numB);
    // }
    // lasta = true;
    attachInterrupt(digitalPinToInterrupt(DT_B_PIN), rotary_dt_B_up, RISING);
    attachInterrupt(digitalPinToInterrupt(CLK_B_PIN), rotary_clk_B_up, RISING);
  }
  /* encoder in 01 state, waiting for B to jump down */
  else {
    detachInterrupt(digitalPinToInterrupt(DT_B_PIN));
    attachInterrupt(digitalPinToInterrupt(CLK_B_PIN), rotary_clk_B_down, FALLING);
  }
}

void rotary_clk_B_down() {
  DEBUGPRINTLN("ROTARY CLK B DOWN");
  /* encoder in 00 state, pin B jumped down last */
  if (digitalRead(DT_B_PIN) == 0) {
    /* for neutral state 00, this is the end of the cycle, but register the clockwise rotation only if pin B jumped last in both half cycles */
    // if (!neutral and !lasta)
    // {
    numB = numB + 1;
    OSCMessage msg("/encB");
    msg.add((int32_t)numB);
    Udp.beginPacket(remoteIP, remotePort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
    DEBUGPRINTLN(numB);
    // }
    // lasta = false;
    attachInterrupt(digitalPinToInterrupt(DT_B_PIN), rotary_dt_B_up, RISING);
    attachInterrupt(digitalPinToInterrupt(CLK_B_PIN), rotary_clk_B_up, RISING);
  }
  /* encoder in 10 state, waiting for A to jump down */
  else {
    attachInterrupt(digitalPinToInterrupt(DT_B_PIN), rotary_dt_B_down, FALLING);
    detachInterrupt(digitalPinToInterrupt(CLK_B_PIN));
  }
}

void rotary_dt_B_up() {
  DEBUGPRINTLN("ROTARY DT B UP");
  /* encoder in 11 state, in A jumped up last */
  if (digitalRead(CLK_B_PIN) == 1) {
    /* for neutral state 11, this is the end of the cycle, but register the counterclockwise rotation only if pin A jumped last in both half cycles */
    // if (neutral and lasta)
    // {
    numB = numB - 1;
    OSCMessage msg("/encB");
    msg.add((int32_t)numB);
    Udp.beginPacket(remoteIP, remotePort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
    DEBUGPRINTLN(numB);
    // }
    // lasta = true;
    attachInterrupt(digitalPinToInterrupt(DT_B_PIN), rotary_dt_B_down, FALLING);
    attachInterrupt(digitalPinToInterrupt(CLK_B_PIN), rotary_clk_B_down, FALLING);
  }
  /* encoder in 10 state, waiting for B to jump up */
  else {
    detachInterrupt(digitalPinToInterrupt(DT_B_PIN));
    attachInterrupt(digitalPinToInterrupt(CLK_B_PIN), rotary_clk_B_up, RISING);
  }
}

void rotary_clk_B_up() {
  DEBUGPRINTLN("ROTARY CLK B UP");
  /* encoder in 11 state, pin B jumped up last */
  if (digitalRead(DT_B_PIN) == HIGH) {
    /* for neutral state 11, this is the end of the cycle, but register the clockwise rotation only if pin B jumped last in both half cycles */
    // if (neutral and !lasta)
    // {
    numB = numB + 1;
    OSCMessage msg("/encB");
    msg.add((int32_t)numB);
    Udp.beginPacket(remoteIP, remotePort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
    DEBUGPRINTLN(numB);
    // }
    // lasta = false;
    attachInterrupt(digitalPinToInterrupt(DT_B_PIN), rotary_dt_B_down, FALLING);
    attachInterrupt(digitalPinToInterrupt(CLK_B_PIN), rotary_clk_B_down, FALLING);
  }
  /* encoder in 01 state, waiting for A to jump up */
  else {
    attachInterrupt(digitalPinToInterrupt(DT_B_PIN), rotary_dt_B_up, RISING);
    detachInterrupt(digitalPinToInterrupt(CLK_B_PIN));
  }
}

void setup() {
#ifdef DEBUG
  //Serial monitor setup
  Serial.begin(115200);

  // Wait for serial to start
  delay(500);
#endif

  // Initialize the pins with the internal pullup
  pinMode(DT_A_PIN, INPUT_PULLUP);
  pinMode(DT_B_PIN, INPUT_PULLUP);
  pinMode(CLK_A_PIN, INPUT_PULLUP);
  pinMode(CLK_B_PIN, INPUT_PULLUP);
  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(WIFI_LED_PIN, OUTPUT);

  // Read the initial states
  int8_t dt_A = digitalRead(DT_A_PIN);
  int8_t clk_A = digitalRead(CLK_A_PIN);
  int8_t dt_B = digitalRead(DT_B_PIN);
  int8_t clk_B = digitalRead(CLK_B_PIN);

  // Initial position of the encoder 11
  if ((dt_A == HIGH) && (clk_A == HIGH)) {
    attachInterrupt(digitalPinToInterrupt(DT_A_PIN), rotary_dt_A_down, FALLING);
    attachInterrupt(digitalPinToInterrupt(CLK_A_PIN), rotary_clk_A_down, FALLING);
    // neutral = 1; /* true */
  }

  // Initial position of the encoder 00
  if ((dt_A == LOW) && (clk_A == LOW)) {
    attachInterrupt(digitalPinToInterrupt(DT_A_PIN), rotary_dt_A_down, RISING);
    attachInterrupt(digitalPinToInterrupt(CLK_A_PIN), rotary_clk_A_up, RISING);
    // neutral = 0; /* false */
  }

  // Initial position of the encoder 11
  if ((dt_B == HIGH) && (clk_B == HIGH)) {
    attachInterrupt(digitalPinToInterrupt(DT_B_PIN), rotary_dt_B_down, FALLING);
    attachInterrupt(digitalPinToInterrupt(CLK_B_PIN), rotary_clk_B_down, FALLING);
    // neutral = 1; /* true */
  }

  // Initial position of the encoder 00
  if ((dt_B == LOW) && (clk_B == LOW)) {
    attachInterrupt(digitalPinToInterrupt(DT_B_PIN), rotary_dt_B_down, RISING);
    attachInterrupt(digitalPinToInterrupt(CLK_B_PIN), rotary_clk_B_up, RISING);
    // neutral = 0; /* false */
  }

  // network connection
  DEBUGPRINT("Attempting to connect to network: ");
  DEBUGPRINTLN(ssid);

  // Set the Wifi mode
  WiFi.mode(WIFI_STA);

  // delete old config
  WiFi.disconnect(true);

  delay(1000);

  WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  WiFi.begin(ssid, pass);
}

void loop() {
  // uint8_t dt_A = digitalRead(DT_A_PIN);
  uint8_t sw = digitalRead(SW_PIN);
  if (sw == LOW) {
    numA=0;
    numB=0;
    OSCMessage msg("/reset");
    Udp.beginPacket(remoteIP, remotePort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
  }
  delay(500);
}
