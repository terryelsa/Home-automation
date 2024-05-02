#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

// Define LoRa module pins
const int csPin = 10;       // LoRa radio chip select (NSS)
const int resetPin = 9;     // LoRa radio reset
const int irqPin = 2;       // Interrupt request pin (DIO0)

// Define relay pins
const int relayPinFan = 5;  // Relay for fan connected to GPIO 5
const int relayPinBulb = 6; // Relay for bulb connected to GPIO 6

// Define DHT sensor setup
#define DHTPIN 7            // DHT11 data pin
#define DHTTYPE DHT11       // DHT11 sensor type
DHT dht(DHTPIN, DHTTYPE);

// Define PIR Motion Sensor pin
const int pirPin = 8;       // PIR motion sensor pin

void setup() {
  Serial.begin(9600);
  pinMode(relayPinFan, OUTPUT);
  pinMode(relayPinBulb, OUTPUT);
  // Ensure relays are OFF by default if using Normally Open
  digitalWrite(relayPinFan, HIGH);
  digitalWrite(relayPinBulb, HIGH);

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

   checkRelayCommands();

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
   
}

void checkRelayCommands() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }
    Serial.print("Command received: ");
    Serial.println(received);
    handleRelayCommands(received);
  }
}

void handleRelayCommands(String command) {
  command.trim(); // Trim any whitespace
  if (command == "FAN ON") {
    digitalWrite(relayPinFan, LOW); // Turn ON the fan
    Serial.println("Fan turned ON");
  } else if (command == "FAN OFF") {
    digitalWrite(relayPinFan, HIGH); // Turn OFF the fan
    Serial.println("Fan turned OFF");
  } else if (command == "BULB ON") {
    digitalWrite(relayPinBulb, LOW); // Turn ON the bulb
    Serial.println("Bulb turned ON");
  } else if (command == "BULB OFF") {
    digitalWrite(relayPinBulb, HIGH); // Turn OFF the bulb
    Serial.println("Bulb turned OFF");
  }
}

void sendSensorData(float temperature, float humidity, int motionDetected) {
  // Prepare sensor data string
  String sensorData = "Temp:" + String(temperature) + ",Humidity:" + String(humidity) + ",Motion:" + String(motionDetected);

  // Send sensor data via LoRa
  LoRa.beginPacket();
  LoRa.print(sensorData);
  LoRa.endPacket();
  Serial.println("Sent sensor data via LoRa: " + sensorData);
}
