/*********
  Rui Santos
*********/

// Import required libraries
#ifdef ESP32
  #include <WiFi.h>
  #include <ESPAsyncWebServer.h>
  #include <SPIFFS.h>
#else
  #include <Arduino.h>
  #include <ESP8266WiFi.h>
  #include <Hash.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>
  #include <FS.h>
#endif
#include <Wire.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "olla-2";
const char* password = "ollaolla";

// Initialize Telegram BOT
#define BOTtoken "1698392996:AAE6aL9qSwMQvoV9eUeXhCCDvsh43h6UN-A"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "993814357"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int ledPin = 2;
bool ledState = LOW;

// defines pins numbers
const int trigPin = 13;
const int echoPin = 12;
//const int buzzer = 27;
long duration;
float distance;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

float readSR04() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

/*digitalWrite(buzzer, HIGH);
delayMicroseconds(400);
digitalWrite(buzzer, LOW);*/

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculating the distance
  distance= 200-(duration*0.034/2);

  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);
  // Read temperature as Celsius (the default)
  float t = distance/100;

  if (isnan(t)) {    
    Serial.println("Failed to read from HC-SR04 sensor!");
  }
  else {
    Serial.println(t);
    return float(t);
  }
}

String waterLevel(){
  float l = readSR04();
  return String(l);
  }

String cond() {
  float h = readSR04();
  if (h <= 1.8) {
    return "AMAN";
    }
   else {
    return "BAHAYA";
    }
  }

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  /*
  bool status; 
  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }*/

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  server.on("/cond", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", cond().c_str());
  });
  server.on("/height", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", waterLevel().c_str());
  });
  /*server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBME280Humidity().c_str());
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBME280Pressure().c_str());
  });*/

  // Start server
  server.begin();
}
 
void loop(){
  float level = readSR04();
  if(level > 1.8){
    bot.sendMessage(CHAT_ID, "Kondisi BAHAYA, Ketinggian Air: " + String(level), "");
  }
}
