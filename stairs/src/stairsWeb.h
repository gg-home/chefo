#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>

const char stairs_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Stairs</title>

    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">

    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js">

    <script src="https://cdn.jsdelivr.net/npm/spectrum-colorpicker2/dist/spectrum.min.js"></script>
    <link rel="stylesheet" type="text/css" href="https://cdn.jsdelivr.net/npm/spectrum-colorpicker2/dist/spectrum.min.css">
  
    <script language="javascript">
        var gwUrl = "ws://" + window.location.hostname + "/stairsws";
        var webSocket = new WebSocket(gwUrl);

        webSocket.onopen = function(e) {
            console.log("open");
        }

        webSocket.onclose = function(e) {
            console.log("close");
        }

        webSocket.onmessage = function(e) {
            console.log(e.data);
        }

        function handleLed() {
            var color = document
                .getElementById('type-color-on-page')
                .value
                .substring(1);

            var brightness = document
                .getElementById('ledBrightness')
                .value;

            webSocket.send(color + "@" + brightness);
        }
    </script>

    <style>
     
    </style>
</head>
<body>
    <h2>Stairs</h2>
    <div class="content">
        <div>
            <p>Pick a color</p>
            <input type="color" id="type-color-on-page" onchange="handleLed()" />
        </div>

        <div>
            <p>Set brightness</p>
            <input id="ledBrightness" type="range" min="1" max="255" value="50" onchange="handleLed()" >
        </div>

    </div>
</body>
</html>
)rawliteral";

// How many leds in your strip?
#define NUM_LEDS 1000
#define DATA_PIN 2

// Define the array of leds
CRGB leds[NUM_LEDS];

class StairsWebClass{

public:
    void begin(AsyncWebServer *server, const char* url = "/stairs"){

        FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
        FastLED.setBrightness(20);
        leds[0] = CRGB::Red;
        FastLED.show();

        _server = server;
        _ws = new AsyncWebSocket("/stairsws");

        _server->on(url, HTTP_GET, [](AsyncWebServerRequest *request){
            request->send_P(200, "text/html", stairs_html, NULL);
        });

        _ws->onEvent([&](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) -> void {
            switch(type) {
                case WS_EVT_CONNECT:
                    Serial.printf("Client connected: \n\tClient id:%u\n\tClient IP:%s\n", 
                        client->id(), client->remoteIP().toString().c_str());
                    break;
                case WS_EVT_DISCONNECT:
                    Serial.printf("Client disconnected:\n\tClient id:%u\n", client->id());
                    break;
                case WS_EVT_DATA:
                    handlingIncomingData(arg, data, len);
                    break;
                case WS_EVT_PONG:
                    Serial.printf("Pong:\n\tClient id:%u\n", client->id());
                    break;
                case WS_EVT_ERROR:
                    Serial.printf("Error:\n\tClient id:%u\n", client->id());
                    break;     
            }
        });

        _server->addHandler(_ws);

        #if defined(DEBUG)
            DEBUG_WEB_SERIAL("Attached AsyncWebServer along with Websockets");
        #endif
    }

private:
    AsyncWebServer *_server;
    AsyncWebSocket *_ws;

    void handlingIncomingData(void *arg, uint8_t *data, size_t len) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;

        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            String cmd = "";
            for (int i=0; i < len; i++)
                cmd += ((char) data[i]);

            Serial.println("Led data: " + cmd);

            String hexColor = "ff0000";
            int brightness = 50;
            for (int i = 0; i < cmd.length(); i++) {
                if (cmd.substring(i, i+1) == "@") {
                    hexColor = cmd.substring(0, i);
                    brightness = cmd.substring(i+1).toInt();
                    break;
                }
            }

            FastLED.setBrightness(brightness);

            int color = strtol(&hexColor[0], NULL, 16);
            leds[0] = color;

            FastLED.show();
        }
    }
    
    #if defined(DEBUG)
        void DEBUG_WEB_SERIAL(const char* message){
            Serial.println("[WebSerial] "+message);
        }
    #endif
};

StairsWebClass StairsWeb;