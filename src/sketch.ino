#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// Wi-Fi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Web server
WebServer server(80);

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// DHT22
#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// Pins
const int gasSensor = 34;
const int ledPin = 25;
const int buzzerPin = 26;

// Sensor values
int gasValue = 0;
float temperature = 0;
float humidity = 0;
String status = "";

// ================= WEB PAGE =================

void handleRoot() {

  String page = "<!DOCTYPE html>";
  page += "<html>";
  page += "<head>";

  page += "<meta name='viewport' content='width=device-width,initial-scale=1'>";

  page += "<meta http-equiv='refresh' content='3'>";

  page += "<title>EcoSense Air Monitor</title>";

  page += "<style>";

  page += "body{font-family:Arial;text-align:center;background:#f2f2f2;margin:0;padding:20px;}";

  page += ".container{max-width:500px;margin:auto;}";

  page += "h1{margin-bottom:5px;}";

  page += ".card{background:white;padding:20px;margin:15px 0;border-radius:15px;box-shadow:0 3px 10px #bbb;}";

  page += ".value{font-size:30px;font-weight:bold;}";

  page += ".status{font-size:25px;font-weight:bold;}";

  page += "</style>";

  page += "</head>";

  page += "<body>";

  page += "<div class='container'>";

  page += "<h1>EcoSense</h1>";

  page += "<p>IoT Air Quality Monitoring System</p>";

  page += "<div class='card'>";

  page += "<h2>🌫️ Gas Level</h2>";

  page += "<div class='value'>" + String(gasValue) + "</div>";

  page += "</div>";

  page += "<div class='card'>";

  page += "<h2>🌡️ Temperature</h2>";

  page += "<div class='value'>" + String(temperature,1) + " °C</div>";

  page += "</div>";

  page += "<div class='card'>";

  page += "<h2>💧 Humidity</h2>";

  page += "<div class='value'>" + String(humidity,1) + " %</div>";

  page += "</div>";

  page += "<div class='card'>";

  page += "<h2>Air Quality</h2>";

  page += "<div class='status'>" + status + "</div>";

  page += "</div>";

  page += "<p>Auto-refresh: 3 seconds</p>";

  page += "</div>";

  page += "</body>";

  page += "</html>";

  server.send(200, "text/html", page);
}


// ================= SETUP =================

void setup() {

  Serial.begin(115200);

  dht.begin();

  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(ledPin, LOW);
  noTone(buzzerPin);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {

    Serial.println("OLED not found!");

    while (1);
  }

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(20,25);

  display.println("EcoSense");

  display.display();

  delay(2000);

  // Wi-Fi
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();

  Serial.println("WiFi Connected!");

  Serial.print("IP Address: ");

  Serial.println(WiFi.localIP());

  // Web server
  server.on("/", handleRoot);

  server.begin();

  Serial.println("Web server started!");
}


// ================= LOOP =================

void loop() {

  // Read sensors

  gasValue = analogRead(gasSensor);

  temperature = dht.readTemperature();

  humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {

    Serial.println("DHT22 reading failed!");

    delay(2000);

    return;
  }

  // Air quality

  if (gasValue < 1500) {

    status = "GOOD";
  }

  else if (gasValue < 2800) {

    status = "MODERATE";
  }

  else {

    status = "POOR";
  }

  // Alert

  if (gasValue >= 2800) {

    digitalWrite(ledPin, HIGH);

    tone(buzzerPin, 2000);
  }

  else {

    digitalWrite(ledPin, LOW);

    noTone(buzzerPin);
  }

  // OLED

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(0,0);

  display.println("AIR QUALITY MONITOR");

  display.setCursor(0,15);

  display.print("Gas: ");

  display.println(gasValue);

  display.setCursor(0,27);

  display.print("Temp: ");

  display.print(temperature,1);

  display.println(" C");

  display.setCursor(0,39);

  display.print("Humidity: ");

  display.print(humidity,1);

  display.println(" %");

  display.setCursor(0,53);

  display.print("Status: ");

  display.println(status);

  display.display();

  // Serial Monitor

  Serial.print("Gas: ");

  Serial.print(gasValue);

  Serial.print(" | Temp: ");

  Serial.print(temperature,1);

  Serial.print(" C | Humidity: ");

  Serial.print(humidity,1);

  Serial.print(" % | Status: ");

  Serial.println(status);

  // Handle web requests

  server.handleClient();

  delay(2000);
}
