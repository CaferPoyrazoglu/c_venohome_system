#include <Arduino.h>
#include <espnow.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SENDER_ID 1
#define DHTPIN 14
#define DHTTYPE DHT11

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  delay(2000);
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
 

  
  WiFi.mode(WIFI_STA);
  WiFi.printDiag(Serial); 
  wifi_promiscuous_enable(1);
  wifi_set_channel(9);
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
     display.clearDisplay();
     display.setCursor(0, 10);
     display.println(dht.readTemperature());

     display.setCursor(0, 30);
     display.println(dht.readHumidity());
     
     display.display();

  
    myData.id = SENDER_ID;
    myData.temp = t;
    myData.hum = h;
    myData.readingId = readingId++;
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  }
}
