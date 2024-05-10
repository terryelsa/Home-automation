#include <ESP8266WiFi.h>
#include <LoRa.h>

// WiFi network details
const char* ssid = " ";
const char* password = " ";

// Blynk authentication token
#define BLYNK_TEMPLATE_ID "TMPL2eJutAnKH"
#define BLYNK_TEMPLATE_NAME "LoRa Ra 02 with ESP8266"
#define BLYNK_AUTH_TOKEN "ZHais_e9FkAFjqzTVa-gX37mCt8dYfeU"

#include <BlynkSimpleEsp8266.h>

BlynkTimer timer;

const int csPin = D8;
const int resetPin = D0;
const int irqPin = D2;

String temperature = "N/A";
String humidity = "N/A";
String motionState = "N/A";


void setup() { 
  Serial.begin(9600);
  // Initialize LoRa
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initializing OK!");

  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
}

void loop() {
  Blynk.run(); // Run Blynk
  timer.run();

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }
    Serial.print("Received from LoRa: "+received);
    parseLoRaMessage(received);
  }
}
void parseLoRaMessage(String received) {
  Serial.println("Received data from LoRa: " + received);

  int tempIndex = received.indexOf("Temp:");
  int humIndex = received.indexOf(",Humidity:");
  int motionIndex = received.indexOf(",Motion:");

  if (tempIndex != -1 && humIndex != -1 && motionIndex != -1) {
    temperature = received.substring(tempIndex + 5, humIndex);
    humidity = received.substring(humIndex + 10, motionIndex);
    int motionStateNum = received.substring(motionIndex + 8).toInt();
    motionState = (motionStateNum == 1) ? "Yes" : "No";

    Serial.println("Temperature: " + temperature);
    Serial.println("Humidity: " + humidity);
    Serial.println("Motion State: " + motionState);

    // Update sensor data on Blynk every 5 seconds
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 5000) {
    lastUpdate = millis();
    // Send sensor data to Blynk
    Blynk.virtualWrite(V0, temperature.toFloat()); // Virtual pin V0 for temperature
    Blynk.virtualWrite(V4, humidity.toFloat());    // Virtual pin V4 for humidity
    Blynk.virtualWrite(V3, motionState == "Yes" ? 1 : 0); // Virtual pin V3 for motion state
  } 
  else {
    Serial.println("Error: Invalid data format");
  }
}
}

BLYNK_WRITE(V1) {
  if (param.asInt() == 1) {
    Serial.println("Button 1 pressed");
    sendLoRaCommand("R1_ON");
  }
  else{
    sendLoRaCommand("R1_OFF"); 
  }
}

BLYNK_WRITE(V2) {
  if (param.asInt() == 1) {
    Serial.println("Button 2 pressed");
    sendLoRaCommand("R2_ON"); 
  }
  else{
     sendLoRaCommand("R2_OFF");
  }
}

void sendLoRaCommand(String command) {
  LoRa.beginPacket();
  LoRa.print(command);
  LoRa.endPacket();
  Serial.println("Sent LoRa command: " + command);
}
