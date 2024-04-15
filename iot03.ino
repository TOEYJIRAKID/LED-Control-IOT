#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

ESP8266WebServer server(80);
DHT dht14(D4, DHT11);

const int ledPin = D6;
bool isLedOn = false;

void init_wifi(String ssid, String password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to " + ssid);
  Serial.println("IP address: " + WiFi.localIP().toString());
}

void setup(void) {
  Serial.begin(115200);
  init_wifi("MAIBOK", "11111111");
  pinMode(ledPin, OUTPUT);
  dht14.begin();

  server.on("/", HTTP_GET, []() {
    float temperature = dht14.readTemperature();
    float humidity = dht14.readHumidity();

    String ledStatus = isLedOn ? "green" : "red";
    String ledText = isLedOn ? "On" : "Off";

    String html = "<html><body style='display: flex; justify-content: center; align-items: center; height: 100vh;'>";
    html += "<div style='text-align: center; background-color: #f5f5f5; border: 5px solid #ccc; padding: 20px; border-radius: 10px; font-family: Arial, sans-serif;'>";
    html += "<p style='font-size: 20px;'>Temperature: <span id='temperature'>" + String(temperature, 2) + " C</span></p>";
    html += "<p style='font-size: 20px;'>Humidity: <span id='humidity'>" + String(humidity, 2) + " %</span></p>";
    html += "<p style='font-size: 20px;'>LED Status: <span style='color: " + ledStatus + "'>" + ledText + "</span></p>"; 
    html += "<div style='display: flex; justify-content: center;'>";
    html += "<form action='/TurnOn' method='get'><input type='submit' value='Turn On'></form>";
    html += "<span style='margin: 0 5px;'></span>";
    html += "<form action='/TurnOff' method='get'><input type='submit' value='Turn Off'></form>";
    html += "</div></div></body></html>";

    html += "<script>setInterval(() => fetch('/TempHum').then(response => response.json()).then(data => { document.getElementById('temperature').textContent = data.temperature.toFixed(2) + ' C'; document.getElementById('humidity').textContent = data.humidity.toFixed(2) + ' %'; }), 500);</script>";

    server.send(200, "text/html", html);
  });

  server.on("/TurnOn", HTTP_GET, []() {
    digitalWrite(ledPin, HIGH);
    isLedOn = true;
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  server.on("/TurnOff", HTTP_GET, []() {
    digitalWrite(ledPin, LOW);
    isLedOn = false;
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  server.on("/TempHum", HTTP_GET, []() {
    float temperature = dht14.readTemperature();
    float humidity = dht14.readHumidity();
    String TempHum = "{\"temperature\":" + String(temperature, 2) + ",\"humidity\":" + String(humidity, 2) + "}";
    server.send(200, "application/json", TempHum);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}