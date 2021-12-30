#include <ESP8266WiFi.h>
#include <espnow.h>
#define RELAY 0 

typedef struct message {
    int role1;
} message;

message msg;

int role1_Durumu = 0;

bool roleDurumDegisimi = true;

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&msg, incomingData, sizeof(msg));
  Serial.print("Relay - 1");
  Serial.println(msg.role1);
  role1_Durumu = msg.role1;
  Serial.println();
  roleDurumDegisimi = true;
}
 
void setup() {
  Serial.begin(9600);
  pinMode(RELAY,OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();        

  if(esp_now_init() != 0) {
    Serial.println("ESP-NOW Baglantisi basarisiz!");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("Basarili!");
}

void loop() {
  if(roleDurumDegisimi)
  {
    digitalWrite(5,role1_Durumu);  
  }
}
