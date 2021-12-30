#include <Arduino.h>
#include <espnow.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define SENDER_ID 1
#define DHTPIN 14
#define DHTTYPE DHT11

float t = 0.0;
float h = 0.0;

DHT dht(DHTPIN, DHTTYPE);

uint8_t broadcastAddress[] = {0x3C, 0x61, 0x05, 0xE4, 0xA8, 0x4F};

typedef struct struct_message {
    int id;
    float temp;
    float hum;
    int readingId;
} struct_message;

struct_message myData;

unsigned long previousMillis = 0;
const long interval = 10000;        

unsigned int readingId = 0;

constexpr char WIFI_SSID[] = "CAFER";

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
    for (uint8_t i=0; i<n; i++) {
      if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
        return WiFi.channel(i);
      }
    }
  }
  return 0;
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Son Gonderilen Paket Durumu: ");
  if (sendStatus == 0){
    Serial.println("Aktarma Basarili!");
  }
  else{
    Serial.println("Aktarma Hatasi!");
  }
}
 
void setup() {
  Serial.begin(9600);
  dht.begin();
  WiFi.mode(WIFI_STA);
  int32_t channel = getWiFiChannel(WIFI_SSID);
  WiFi.printDiag(Serial); 
  wifi_promiscuous_enable(1);
  wifi_set_channel(channel);
  wifi_promiscuous_enable(0);
  WiFi.printDiag(Serial);

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW iletisimi basarisiz!");
    return;
  }
  
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}
 
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    float newT = dht.readTemperature();
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
      Serial.println(t);
    }
    float newH = dht.readHumidity();
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;
      Serial.println(h);
    }
    myData.id = SENDER_ID;
    myData.temp = t;
    myData.hum = h;
    myData.readingId = readingId++;
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  }
}
