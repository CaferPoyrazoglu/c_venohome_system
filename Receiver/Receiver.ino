#include <ESP8266WiFi.h>
#include <espnow.h>

uint8_t broadcastAddress[] = {0xC4, 0x5B, 0xBE, 0x67, 0x9C, 0xEF};

typedef struct message  {
 int relay1;
} message ;

message  myData;

int temp = 0;

String success;


// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
 
}

 
void setup() {
  Serial.begin(9600);
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
    if(temp == 2){
      temp = 0;
    }
    
    myData.relay1 = temp;
    temp++;
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    delay(2000);
  }
