#include <WiFi.h>
#include <HTTPClient.h>

#include "secrets.h"

HTTPClient http;

void setup() {
  Serial.begin(115200);
  delay(4000);

  WiFi.begin(WIFI_SSID, WIFI_PSWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi");
  }
  Serial.println("Connected to WiFi");

 
}

void loop() {
  if(WiFi.status() == WL_CONNECTED) {

    http.begin("http://jsonplaceholder.typicode.com/posts");
    http.addHeader("Content-Type", "text/plain");

    int httpResponseCode= http.POST("POSTING from BS's ESP32");

    if(httpResponseCode > 0) {
      String response = http.getString();

      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error on send: ");
      Serial.println(httpResponseCode);
    }
     http.end();
  } else {
    Serial.print("Error in WiFi connection");
  }
  delay(10000);
}
