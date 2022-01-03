#include <ESP8266WiFi.h>
#include <espnow.h>

uint8_t broadcastAddress[] = {0xC4, 0x5B, 0xBE, 0x67, 0x9C, 0xEF};

typedef struct message  {
 int relay1;
} message ;

message  myData;

int temp = 0;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Durum: ");
  if (sendStatus == 0){
    Serial.println("Aktarma Basarili");
  }
  else{
    Serial.println("Aktarma Hatasi!");
  }
}
 
void setup() {
  Serial.begin(9600);
 
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW Hatasi!");
    return;
  }
  
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}
 
void loop() {
    if(temp == 2){
      temp = 0;
    }
    
    myData.relay1 = temp;
    temp++;
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    delay(2000);
  }
