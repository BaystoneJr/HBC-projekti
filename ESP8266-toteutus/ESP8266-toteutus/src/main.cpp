#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include "secrets.h"
#include "mhz19.h"


static char esp_id[16];

HTTPClient http;

//CO2 koodin alku
static bool exchange_command(uint8_t cmd, uint8_t data[], int timeout) {
  // create command buffer
  uint8_t buf[9];
  int len = prepare_tx(cmd, data, buf, sizeof(buf));

  // send the command
  sensor.write(buf, len);

  // wait for response
  long start = millis();
  while ((millis() - start) < timeout) {
    if (sensor.available() > 0) {
      uint8_t b = sensor.read();
      if (process_rx(b, cmd, data)) {
        return true;
      }
    }
  }
  return false;
}

static bool read_temp_co2(int *co2, int *temp) {
  uint8_t data[] = {0, 0, 0, 0, 0, 0};
  bool result = exchange_command(0x86, data, 3000);
  if (result) {
    *co2 = (data[0] << 8) + data[1];
    *temp = data[2] - 40;
    char raw[32];
    sprintf(raw, "Raw co2 sensor data: %02X %02X %02X %02X %02X %02X", data[0], data[1], data[2], data[3], data[4], data[5]);
    Serial.println(raw);
  }
  return result;
}
//CO2 koodin loppu

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

    float Temp;
    float Hum;
    int co2, tempMHZ;

    Temp = dht.readTemperature();
    Hum = dht.readHumidity();

    read_temp_co2(&co2, &tempMHZ);

    Serial.println("*************************************");
    Serial.print("CO2: ");
    Serial.println(co2, DEC);
    Serial.print("Temp(MHZ19, vertailukelpoista): ");
    Serial.print(tempMHZ);
    Serial.println("Temp(DHT): ");
    Serial.print(Temp, 2);
    Serial.println("Humidity: ");
    Serial.print(hum, 2);
    Serial.println("*************************************");

    if(!read_temp_co2(&co2, &tempMHZ)) {
        Serial.println("MHZ19 read failed;")
        co2 = 0;
        tempMHZ = 0;
        return;
    }
    if(isnan(Temp) || isnan(Hum)) {
        Serial.println("Read of Temperature or Humidity from DHT failed");
        Temp = 0;
        Hum = 0;
    }
    //kirjoitetaan arvot, jos mittaukset virheellisiä/tyhjiä(keksi parempi null tarvittaessa myöhemmin)

    char jsonMessage[256]; //kokoa voidaan pienentää tarvittaessa(vaikka tiedon koon ei pitäisi missään tilanteessa olla ongelma)

    DynamicJsonBuffer jsonBuffer(256); //sama tässä kuin ylemmässä kommentissa, tosin dynaaminen, eli käyttää tilaa vain sen verran, kuin tarvitsee
    JsonObject& sendedJson = jsonBuffer.createObject();

    sendedJson["ESP_ID"] = esp_id;
    //lisää tähän kenttiä tarvittaessa

    JsonObject& data = sendedJson.createNestedOobject("data");
    data["CO2"] = co2;
    data["Temp"] = Temp;
    data["Hum"] = Hum;
    //lisää tähän kenttiä tarvittaessa

    Serial.println("Sending JSON....");
    root.prettyPrintTo(Serial);
    root.printTO(jsonMessage);
        


    http.begin(HostIP);
    http.addHeader("Content-Type", "application/json"); //muuta myöhemmin esim. application/json
    
    int httpCode = http.POST(jsonMessage);
    String payload = http.getString();

    Serial.println(httpCode);
    Serial.println(payload);

    http.end();

    
}