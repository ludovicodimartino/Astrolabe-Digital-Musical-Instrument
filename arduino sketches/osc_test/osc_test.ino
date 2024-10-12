#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;        // network SSID (name)
char pass[] = SECRET_PASS;    // network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status

WiFiUDP Udp;                                // A UDP instance to let us send and receive packets over UDP
const IPAddress outIp(192,168,43,238);        // remote IP of your computer
const unsigned int outPort = 9999;          // remote port to receive OSC
const unsigned int localPort = 8888;        // local port to listen for OSC packets (actually not used for sending)


void setup() {
  //Serial monitor setup
  Serial.begin(9600);

  /* Wait for the Serial Monitor */
  while (!Serial) {
    delay(1);
  }

  Serial.print("Attempting to connect to network: ");
  Serial.println(ssid);
  
  // Connect to WPA/WPA2 network
  status = WiFi.begin(ssid, pass);
  Serial.println("Connecting...");

  // attempt to connect to Wifi network
  while (status != WL_CONNECTED) {
    Serial.println(".");
    // wait 10 seconds for connection
    delay(500);
  }

  // you're connected now, so print out the data:
  Serial.println("Connection succeed!");
  Serial.println("----------------------------------------");
  printWifiData();
  Serial.println("----------------------------------------");

  Serial.println("Starting UDP");
    Udp.begin(localPort);
    Serial.print("Local port: ");
}

void loop() {
  OSCMessage msg("/test");
  msg.add("hello, osc.");
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
  delay(2000); // wait 2 secons
}

void printWifiData() {
  Serial.println("Board Information:");
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
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}