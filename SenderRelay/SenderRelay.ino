#include <ESP8266WiFi.h>
#include <espnow.h>
#define RELAY 0 

uint8_t broadcastAddress[] = {0x3C, 0x61, 0x05, 0xE4, 0xA8, 0x4F};
int role1_Durumu = 0;

typedef struct message {
    int role1;
} message;

message msg;


void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&msg, incomingData, sizeof(msg));
  Serial.print("Relay - 1");
  Serial.println(msg.role1);
  role1_Durumu = msg.role1;
  Serial.println();

}

 
void setup() {
  Serial.begin(9600);
  pinMode(RELAY,OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
  if(role1_Durumu == 1)
  {
    digitalWrite(RELAY,HIGH);  
  }else{
    digitalWrite(RELAY,LOW); 
  }
}
