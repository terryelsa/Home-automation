#include <SPI.h>
#include <LoRa.h>

// Define LoRa module pins
const int csPin = 10;    // LoRa radio chip select (NSS)
const int resetPin = 9;  // LoRa radio reset
const int irqPin = 2;    // Interrupt request pin (DIO0)

// Define relay pins
#define relayPin1 5  // Relay for fan connected to GPIO 5
#define relayPin2 6  // Relay for bulb connected to GPIO 6

void setup() {
  Serial.begin(9600);
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  digitalWrite(relayPin1, HIGH);  // Ensure relays are OFF by default
  digitalWrite(relayPin2, HIGH);

  // Initialize LoRa
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
  Serial.println("LoRa Receiver Ready!");
}

void loop() {
  receiveLoRaCommand();
  Serial.println("Sent sensor data via LoRa");
}


void receiveLoRaCommand() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String receivedCommand = LoRa.readString();
    Serial.print("Received command: ");
    Serial.println(receivedCommand);

    if (receivedCommand == "R1_ON") {
      digitalWrite(relayPin1, LOW);  // Turn on relay 1
      Serial.println("Relay 1 ON");
    } else if (receivedCommand == "R1_OFF") {
      digitalWrite(relayPin1, HIGH);  // Turn off relay 1
      Serial.println("Relay 1 OFF");
    } else if (receivedCommand == "R2_ON") {
      digitalWrite(relayPin2, LOW);  // Turn on relay 2
      Serial.println("Relay 2 ON");
    } else if (receivedCommand == "R2_OFF") {
      digitalWrite(relayPin2, HIGH);  // Turn off relay 2
      Serial.println("Relay 2 OFF");
    }
  }
}
