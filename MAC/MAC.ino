
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif

void setup(){
  Serial.begin(9600);
  Serial.println();
  Serial.print("MAC Addresi:  ");
  Serial.println(WiFi.macAddress());
}
 
void loop(){

}
