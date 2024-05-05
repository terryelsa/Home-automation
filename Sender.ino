#include <ESP8266WiFi.h>
#include <LoRa.h>

// WiFi network details
const char* ssid = "mike265";
const char* password = "773aee3ccf05c";

const int csPin = D8;
const int resetPin = D0;
const int irqPin = D2;

WiFiServer server(80);

String temperature = "N/A";
String humidity = "N/A";
String motionState = "N/A";

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  
  // Initialize LoRa
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String httpRequest = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        httpRequest += c;
        if (c == '\n' && httpRequest.endsWith("\r\n\r\n")) {
          Serial.print("HTTP Request: ");
          Serial.println(httpRequest);
          handleClientRequest(httpRequest, client);
          client.stop();
          httpRequest = "";
        }
      }
    }
  }

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }
    Serial.print("Received from LoRa: ");
    Serial.println(received);
    parseLoRaMessage(received);
  }
}

void handleClientRequest(String request, WiFiClient& client) {
  String responseHTML = "<!DOCTYPE html><html><head><title>Home Automation System</title>"
                        "<style>"
                        "body { text-align: center; }"
                        ".button {"
                        "  display: inline-block;"
                        "  padding: 10px 20px;"
                        "  border-radius: 20px;"
                        "  text-decoration: none;"
                        "  color: white;"
                        "  font-weight: bold;"
                        "  cursor: pointer;"
                        "}"
                        ".button-on {"
                        "  background-color: red;"
                        "}"
                        ".button-off {"
                        "  background-color: blue;"
                        "}"
                        "</style>"
                        "</head><body><div>"
                        "<h1>Home Automation System</h1>"
                        "<h2>Sensor Data:</h2>"
                        "<p>Temperature: " + temperature + "&deg C</p>"
                        "<p>Humidity: " + humidity + "%</p>"
                        "<p>Motion Detected: " + motionState + "</p>"
                        "<h2>Device Control:</h2>"
                        "<div>"
                        "<a class=\"button button-on\" href=\"/fan/on\">Turn Fan ON</a>"
                        "<a class=\"button button-off\" href=\"/fan/off\">Turn Fan OFF</a>"
                        "</div>"
                        "<div>"
                        "<a class=\"button button-on\" href=\"/bulb/on\">Turn Bulb ON</a>"
                        "<a class=\"button button-off\" href=\"/bulb/off\">Turn Bulb OFF</a>"
                        "</div>"
                        "</div></body></html>";

  if (request.indexOf("GET /fan/on")!= -1) {
    LoRa.beginPacket();
    LoRa.print("FAN ON");
    LoRa.endPacket();
  } else if (request.indexOf("GET /fan/off")!= -1) {
    LoRa.beginPacket();
    LoRa.print("FAN OFF");
    LoRa.endPacket();
  } else if (request.indexOf("GET /bulb/on")!= -1) {
    LoRa.beginPacket();
    LoRa.print("BULB ON");
    LoRa.endPacket();
  } else if (request.indexOf("GET /bulb/off")!= -1) {
    LoRa.beginPacket();
    LoRa.print("BULB OFF");
    LoRa.endPacket();
  }                      

  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  client.println(responseHTML);
}

void parseLoRaMessage(String received) {
  Serial.println("Received data from LoRa: " + received);

  if (received.startsWith("Temp:") && received.indexOf(",Humidity:") != -1 && received.indexOf(",Motion:") != -1) {
    int tempIndex = received.indexOf("Temp:") + 5;
    int humIndex = received.indexOf(",Humidity:");
    int motionIndex = received.indexOf(",Motion:");

    if (tempIndex != -1 && humIndex != -1 && motionIndex != -1) {
      temperature = received.substring(tempIndex, humIndex);
      humidity = received.substring(humIndex + 10, motionIndex);
      int motionStateNum = received.substring(motionIndex + 8).toInt();
      motionState = (motionStateNum == 1) ? "Yes" : "No";

      Serial.println("Temperature: " + temperature);
      Serial.println("Humidity: " + humidity);
      Serial.println("Motion State: " + motionState);
    } else {
      Serial.println("Error: Invalid format");
    }
  } else {
    Serial.println("Error: Invalid data format");
  }
}
