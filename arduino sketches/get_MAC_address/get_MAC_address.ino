/*
  This code is used to get the MAC address of the ESP-01S master.
  The MAC address will be used by the slaves to communicate with the master. 
*/
#include <ESP8266WiFi.h>

void setup(){
  Serial.begin(115200);
  Serial.print("ESP Board MAC Address: ");
  Serial.println(WiFi.macAddress());
}
 
void loop(){

}