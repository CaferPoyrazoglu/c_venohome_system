#include <espnow.h>
#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include "ESPAsyncTCP.h"
#include <Arduino_JSON.h>
#define RELAY_NO true

const char* ssid = "CAFER";
const char* password = "poyraz.1963";

const char* PARAM_INPUT_1 = "relay";  
const char* PARAM_INPUT_2 = "state";



uint8_t role1Mac[] = {0xC4, 0x5B, 0xBE, 0x67, 0x9C, 0xEF};
uint8_t role2Mac[] = {0xC4, 0x5B, 0xC4, 0x67, 0xC4, 0x67};

typedef struct message  {
 int relay1;
} message ;

typedef struct struct_message {
  int id;
  float temp;
  float hum;
  unsigned int readingId;
} struct_message;

struct_message incomingReadings;
message  myData;

JSONVar board;

AsyncWebServer server(80);
AsyncEventSource events("/events");

int temp = 0;

String success;

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

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("PAKET DURUMU: ");
  if (sendStatus == 0){
    Serial.println("Paket Basariyla Gonderildi!");
  }
  else{
    Serial.println("Hata!");
  }
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
            background-color: #314568;
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
            width: 120px;
            height: 68px
        }

        .switch input {
            display: none
        }

        .slider {
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            border-radius: 34px
        }

        .slider:before {
            position: absolute;
            content: "";
            height: 52px;
            width: 52px;
            left: 8px;
            bottom: 8px;
            background-color: #fff;
            -webkit-transition: .4s;
            transition: .4s;
            border-radius: 68px
        }

        input:checked+.slider {
            background-color: #2196F3
        }

        input:checked+.slider:before {
            -webkit-transform: translateX(52px);
            -ms-transform: translateX(52px);
            transform: translateX(52px)
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

            <h4>Relay 1</h4>
            <label class="switch">
                <input type="checkbox" onchange="toggleCheckbox(this)" id="1"><span class="slider"></span>
            </label>

            <h4>Relay 2</h4>
            <label class="switch">
                <input type="checkbox" onchange="toggleCheckbox(this)" id="2"><span class="slider"></span>
            </label>

        </div>
    </div>
 

      <script>
      
        function toggleCheckbox(element) {
            var xhr = new XMLHttpRequest();
            if (element.checked) {
                xhr.open("GET", "/update?relay=" + element.id + "&state=1", true);
            } else {
                xhr.open("GET", "/update?relay=" + element.id + "&state=0", true);
            }
            xhr.send();
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

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(role1Mac, ESP_NOW_ROLE_COMBO, WiFi.channel(), NULL, 0);
  esp_now_add_peer(role2Mac, ESP_NOW_ROLE_COMBO, WiFi.channel(), NULL, 0);
  esp_now_register_recv_cb(OnDataRecv);

   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

   server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    String inputMessage2;
    String inputParam2;
    int roleNo;
    // GET input1 value on <ESP_IP>/update?relay=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1) & request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;
   
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }

    if(inputParam + inputMessage == "relay1"){
      
      Serial.println(inputParam + inputMessage + " - " + "Yeni Durum:" + inputMessage2);
      myData.relay1 = inputMessage2.toInt();
      esp_now_send(role1Mac, (uint8_t *) &myData, sizeof(myData));
      
    } 
    
    if (inputParam + inputMessage == "relay2"){

      Serial.println(inputParam + inputMessage + " - " + "Yeni Durum:" + inputMessage2);
      myData.relay1 = inputMessage2.toInt();
      esp_now_send(role2Mac, (uint8_t *) &myData, sizeof(myData));
      
    }
   
    
    
    
    request->send(200, "text/plain", "OK");
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
   
  }
