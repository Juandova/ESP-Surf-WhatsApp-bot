/*
  Project details:
  https://RandomNerdTutorials.com/esp8266-nodemcu-https-requests/ 
  Basic ESP8266Wifi library explanation: 
  https://esp8266-arduino-spanish.readthedocs.io/es/latest/esp8266wifi/readme.html
  Configuring WhatsApp bot:
  https://randomnerdtutorials.com/esp8266-nodemcu-send-messages-whatsapp/
  Wave API:
  https://open-meteo.com/
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <UrlEncode.h>

// Adjustable Variables
#define HOURS 12
#define MIN_WAVES_MED 0.7
String phoneNumber = "+xxYourphonenumber";
String apiKey = "yourapikey";
const char* ssid = "yourssid";
const char* password = "ssidpassword";

unsigned long sleepDuration = HOURS * 60 * 60 * 1000;  //From Hours to milliseconds

//Fixed Variables
int i = 0;
String m, n;
int today_waves = 0;
int weekly_waves = 0;

void sendMessage(String message){

  // Data to send with HTTP POST
  String url = "http://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
  Serial.println(url);
  WiFiClient client;    
  HTTPClient http;
  http.begin(client, url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.print("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

void setup() { 
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  Serial.println();
  Serial.println();
  Serial.println();

}

void loop() {
  today_waves = 0;
  weekly_waves = 0;
  m = "*Waves have been detected today:*\n";
  n = "*Waves have been detected this week:*\n";
  JsonDocument doc;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
    yield();//Just in case
  }
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {
    
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    // Ignore SSL certificate validation
    client->setInsecure();
    
    //create an HTTPClient instance
    HTTPClient https;
    
    //Initializing an HTTPS communication using the secure client
    Serial.print("[HTTPS] begin...\n");
    //Replace the HTTPS with your adjustsments
    if (https.begin(*client, "https://marine-api.open-meteo.com/v1/marine?latitude=39.4&longitude=-0.3000002&current=wave_height&hourly=wave_height,wave_period&daily=wave_height_max&timezone=Europe%2FBerlin&forecast_hours=24")) {  // HTTPS
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);

      deserializeJson(doc, payload);
      for(i=0;i<7;i++){
        String dia = doc["daily"]["time"][i];
        float altura_max = doc["daily"]["wave_height_max"][i]; // Takes the wave values
        n = n + "*" +  String(dia) + "* " + "\nHeight: " + String(altura_max) +"m\n";
        if(altura_max >= MIN_WAVES_MED){weekly_waves = 1;}
      }
      for(i=0;i<15;i++){
        String hora = doc["hourly"]["time"][i]; // Takes the timestamp Ej: 2024-02-04T15:00
        float altura = doc["hourly"]["wave_height"][i]; // Takes the wave height
        float periodo = doc["hourly"]["wave_period"][i]; // Takes the period
        m = m + "*" +  String(hora) + "* \n" + "Height:" + String(altura) + " " + "Period:" + String(periodo) + "\n";
        if(altura >= MIN_WAVES_MED){today_waves = 1;}
      }
      if(today_waves == 1){sendMessage(m);}
      delay(5000);
      if(weekly_waves == 1){sendMessage(n);}
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  delay(10000);
  WiFi.disconnect();
  Serial.println("Disconnecting WiFi");
  unsigned long startTime = millis();
  while ((millis() - startTime) < sleepDuration) {// Not really efficient but it works 12 horas son 43200000 | 6 horas son 21600000
    Serial.println(".");
  }  
  //ESP.deepSleep(1 * 60 * 60 * 1000000, WAKE_RF_DEFAULT); // Discarded method
  //delay(21600000);// Discarded method
}
