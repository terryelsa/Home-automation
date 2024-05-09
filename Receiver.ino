#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

// Define LoRa module pins
const int csPin = 10;       // LoRa radio chip select (NSS)
const int resetPin = 9;     // LoRa radio reset
const int irqPin = 2;       // Interrupt request pin (DIO0)

// Define relay pins
#define relayPin1 5 // Relay for fan connected to GPIO 5
#define relayPin2 6 // Relay for bulb connected to GPIO 6

// Define DHT sensor setup
#define DHTPIN 7            // DHT11 data pin
#define DHTTYPE DHT11       // DHT11 sensor type
DHT dht(DHTPIN, DHTTYPE);

// Define PIR Motion Sensor pin
const int pirPin = 8;       // PIR motion sensor pin

void setup() {
  Serial.begin(9600);
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  // Ensure relays are OFF by default if using Normally Open
  digitalWrite(relayPin1, HIGH);
  digitalWrite(relayPin2, HIGH);

  // Initialize LoRa
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Receiver Ready");

  // Initialize DHT sensor
  dht.begin();

  // Initialize PIR motion sensor
  pinMode(pirPin, INPUT);
}

void loop() {
  // Read sensor data
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int motionDetected = digitalRead(pirPin);

  // Print sensor readings
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, Humidity: ");
  Serial.print(humidity);
  Serial.print("%, Motion Detected: ");
  Serial.println(motionDetected ? "Yes" : "No");

  // Send sensor data via LoRa to transmitter
  sendSensorData(temperature, humidity, motionDetected);

  // Check for LoRa commands
  receiveLoRaCommand();
}

void sendSensorData(float temperature, float humidity, int motionDetected) {
  LoRa.beginPacket();
  LoRa.print("Temp:");
  LoRa.print(temperature);
  LoRa.print(",Humidity:");
  LoRa.print(humidity);
  LoRa.print(",Motion:");
  LoRa.println(motionDetected ? "1" : "0");
  LoRa.endPacket();
  Serial.println("Sent sensor data via LoRa");
}

void receiveLoRaCommand() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String receivedCommand = LoRa.readString();
    Serial.println("Received command: " + receivedCommand);

    // Process the received command
    if (receivedCommand == "R1_ON") {
      digitalWrite(relayPin1, LOW); // Turn on relay 1
    } else if (receivedCommand == "R1_OFF") {
      digitalWrite(relayPin1, HIGH); // Turn off relay 1
    }
    else if (receivedCommand == "R2_ON") {
      digitalWrite(relayPin2, LOW); // Turn on relay 2
    } else if (receivedCommand == "R2_OFF") {
      digitalWrite(relayPin2, HIGH); // Turn off relay 2
    }
  }
}
