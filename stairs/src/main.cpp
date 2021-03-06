#include <Arduino.h>
#include <Credentials.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>
#include <OTA.h>
#include <StairsWeb.h>


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Hello</title>

    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
</head>
<body>
    <h1>Home</h1>
    <a href="./stairs">Stairs</a>
</body>
</html>
)rawliteral";

// Web server running on port 80
AsyncWebServer server(80);

void recvMsg(uint8_t *data, size_t len){
    String d = "";
    for(int i=0; i < len; i++){
        d += char(data[i]);
    }
    WebSerial.println(d);
}

void setup() {
    Serial.begin(9600);
    
    setupOTA("ChefoStairs", WIFI_SSID, WIFI_PW);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html, NULL);
    });

    StairsWeb.begin(&server);
    Stairs.initialize();

    WebSerial.begin(&server);
    WebSerial.msgCallback(recvMsg);

    server.begin();
}

void loop() {
    ArduinoOTA.handle();

    Stairs.live();
}