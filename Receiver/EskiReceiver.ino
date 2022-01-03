#include <espnow.h>
#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include "ESPAsyncTCP.h"
#include <Arduino_JSON.h>

const char* ssid = "BSC";
const char* password = "32bsc2021";

typedef struct struct_message {
  int id;
  float temp;
  float hum;
  unsigned int readingId;
} struct_message;

struct_message incomingReadings;

JSONVar board;

AsyncWebServer server(80);
AsyncEventSource events("/events");

void OnDataRecv(uint8_t * mac_addr, uint8_t *incomingData, uint8_t len) { 
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  
  board["id"] = incomingReadings.id;
  board["temperature"] = incomingReadings.temp;
  board["humidity"] = incomingReadings.hum;
  board["readingId"] = String(incomingReadings.readingId);
  String jsonString = JSON.stringify(board);
  events.send(jsonString.c_str(), "new_readings", millis());
  
  Serial.printf("Board ID %u: %u bytes\n", incomingReadings.id, len);
  Serial.printf("t value: %4.2f \n", incomingReadings.temp);
  Serial.printf("h value: %4.2f \n", incomingReadings.hum);
  Serial.printf("readingID value: %d \n", incomingReadings.readingId);
  Serial.println();
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
    <title>Small Paket Dashboard</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" type="text/css" href="//fonts.googleapis.com/css?family=Varela+Round" />
    <link rel="icon" href="data:,">
    <style>
        html {
            font-family: Futura, "Trebuchet MS", Arial, sans-serif;
            font-size: 20px;
            font-style: normal;
            font-variant: normal;
            font-weight: 500;
            line-height: 26.4px;
            display: inline-block;
            text-align: center;
        }
        h1 {
            font-size: 2rem;
        }
        body {
            margin: 0;
        }
        .topnav {
            overflow: hidden;
            background-color: #2f4468;
            color: white;
            font-size: 1.7rem;
        }
        .content {
            padding: 20px;
        }
        .card {
            background-color: rgb(255, 255, 255);
            box-shadow: 2px 2px 24px 1px rgb(180, 180, 180);
            border-radius: 16px;
        }
        .cards {
            max-width: 700px;
            margin: 0 auto;
            display: grid;
            grid-gap: 2rem;
            grid-template-columns: repeat(auto-fit, minmax(320px, 1fr));
        }
        .reading {
            font-size: 36px;
            font-weight: 500;
        }
        .timestamp {
            color: #000000;
            font-size: 16px;
            font-weight: 400;
        }
        .card-title {
            font-size: 22px;
            font-weight: bold;
        }
        .card.temperature {
            color: #000000;
        }
        .card.humidity {
            color: #50B8B4;
        }
        .switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
        }
        /* Hide default HTML checkbox */
        .switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        /* The slider */
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            -webkit-transition: .4s;
            transition: .4s;
        }
        .slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            -webkit-transition: .4s;
            transition: .4s;
        }
        input:checked+.slider {
            background-color: #2196F3;
        }
        input:focus+.slider {
            box-shadow: 0 0 1px #2196F3;
        }
        input:checked+.slider:before {
            -webkit-transform: translateX(26px);
            -ms-transform: translateX(26px);
            transform: translateX(26px);
        }
        /* Rounded sliders */
        .slider.round {
            border-radius: 34px;
        }
        .slider.round:before {
            border-radius: 50%;
        }
    </style>
</head>
<body style="background-color: rgb(231, 231, 231);">
    <div class="content">
        <div class="cards">
            <div class="card temperature">
                <p class="card-title"><i class="fas fa-thermometer-half"></i> Sicaklik</p>
                <p><span class="reading"><span id="t1"></span> &deg;C</span></p>
                <p class="timestamp">Son Okunma Zamani</p>
                <p class="timestamp"><span id="d1"></span></p>
            </div>
            <div class="card temperature">
                <p class="card-title"><i class="fas fa-thermometer-half"></i> Nem</p>
                <p><span class="reading"><span id="h1"></span> &deg;%</span></p>
                <p class="timestamp">Son Okunma Zamani</p>
                <p class="timestamp"><span id="d2"></span></p>
            </div>
            <div class="card temperature">
                <p class="card-title"><i class="fas fa-thermometer-half"></i> Role - 1</p>
                <p><span class="reading"><span id="rly1"></span> <label class="switch">
                            <input type="checkbox">
                            <span class="slider round"></span>
                        </label></span></p>
            </div>
        </div>
    </div>
    <script>
        function getDateTime() {
            var currentdate = new Date();
            var datetime = currentdate.getDate() + "/" +
                (currentdate.getMonth() + 1) + "/" +
                currentdate.getFullYear() + " at " +
                currentdate.getHours() + ":" +
                currentdate.getMinutes() + ":" +
                currentdate.getSeconds();
            return datetime;
        }
        if (!!window.EventSource) {
            var source = new EventSource('/events');
            source.addEventListener('open', function (e) {
                console.log("Events Connected");
            }, false);
            source.addEventListener('error', function (e) {
                if (e.target.readyState != EventSource.OPEN) {
                    console.log("Events Disconnected");
                }
            }, false);
            source.addEventListener('message', function (e) {
                console.log("message", e.data);
            }, false);
            source.addEventListener('new_readings', function (e) {
                console.log("new_readings", e.data);
                var obj = JSON.parse(e.data);
                document.getElementById("t1").innerHTML = obj.temperature.toFixed(1);
                document.getElementById("h1").innerHTML = obj.humidity.toFixed(1);
                document.getElementById("d1").innerHTML = getDateTime();
                document.getElementById("d2").innerHTML = getDateTime();
            }, false);
        }
    </script>
</body>
</html>)rawliteral";

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb(OnDataRecv);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
   
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
 
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin();
}
 
void loop() {
  static unsigned long lastEventTime = millis();
  static const unsigned long EVENT_INTERVAL_MS = 5000;
  if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
    events.send("ping",NULL,millis());
    lastEventTime = millis();
  }
}