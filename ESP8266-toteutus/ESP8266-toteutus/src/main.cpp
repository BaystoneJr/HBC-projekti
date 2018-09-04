#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"
#include "mhz19.h"

static char esp_id[16];

HTTPClient http;



void setup() {
    Serial.begin(115200);
    connectToWifi();
    
    sprintf(esp_id, "%08X", ESP.getChipId());
    Serial.print("ESP ID: ");
    Serial.println(esp_id);
}

void loop() {
    if(WiFi.status() != WL_CONNECTED) {
        connectToWifi();
    }
    if(WiFi.status() == WL_CONNECTED) {
        sendToAPI();
    }

}

void connectToWifi() {
    int count = 0;

    WiFi.begin(WIFI_SSID, WIFI_PSWD);
    
    while(WiFi.status() != WL_CONNECTED) {
        delay(1000);
        count++;
        Serial.println("Connecting....");
    }
    if(count == 20) {
        ESP.restart();
    }

    Serial.println("Connected to WiFI.");
}

void sendToAPI() {
    
    char jsonMessage[256]; //kokoa voidaan pienentää tarvittaessa(vaikka tiedon koon ei pitäisi missään tilanteessa olla ongelma)

    DynamicJsonBuffer jsonBuffer(256); //sama tässä kuin ylemmässä kommentissa
    JsonObject& sendedJson = jsonBuffer.createObject();

    sendedJson["ESP_ID"] = esp_id;

    
    


    http.begin(HostIP);
    http.addHeader("Content-Type", "application/json"); //muuta myöhemmin esim. application/json
    
    int httCode = http.POST(jsonMessage);

    



    
}