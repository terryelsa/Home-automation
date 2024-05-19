#include <ESP8266WiFi.h>
#include <LoRa.h>

// WiFi network details
const char* ssid = "Your WiFi name";
const char* password = "Your WiFi password";
// Blynk authentication token
#define BLYNK_TEMPLATE_ID "TMPL2eJutAnKH"
#define BLYNK_TEMPLATE_NAME "LoRa Ra 02 with ESP8266"
#define BLYNK_AUTH_TOKEN "ZHais_e9FkAFjqzTVa-gX37mCt8dYfeU"

#include <BlynkSimpleEsp8266.h>

// Define push button pins
const int buttonPin1 = D1;  // Connect the push button for relay 1 to GPIO D1
const int buttonPin2 = D3;  // Connect the push button for relay 2 to GPIO D3

const int csPin = D8;
const int resetPin = D0;
const int irqPin = D2;

// Counters for button presses
int buttonPressCount1 = -1;  // Initialize to -1 so the first press sets to 0 and sends "ON"
int buttonPressCount2 = -1;  // Initialize to -1 so the first press sets to 0 and sends "ON"

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin1, INPUT_PULLUP);  // Internal pull-up resistor
  pinMode(buttonPin2, INPUT_PULLUP);  // Internal pull-up resistor

  // Initialize LoRa
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
  Serial.println("LoRa Initializing OK!");

  // Attempt to connect to WiFi
  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();

  // Only try to connect for 10 seconds
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    Serial.print(".");
    delay(500);
  }

  // Initialize Blynk if WiFi is connected
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
    Serial.println("WiFi connected");
  } else {
    Serial.println("WiFi connection failed. Manual control only.");
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
  }
  manualControl();
}

void manualControl() {
  static unsigned long lastDebounceTime1 = 0;
  static unsigned long lastDebounceTime2 = 0;
  const unsigned long debounceDelay = 50;  // Adjust debounce delay as necessary

  static int buttonState1 = HIGH;      // Initial state of button 1
  static int lastButtonState1 = HIGH;  // Previous state of button 1

  static int buttonState2 = HIGH;      // Initial state of button 2
  static int lastButtonState2 = HIGH;  // Previous state of button 2

  // Read the state of the push button 1
  int reading1 = digitalRead(buttonPin1);

  // If the switch changed, due to noise or pressing:
  if (reading1 != lastButtonState1) {
    // reset the debouncing timer
    lastDebounceTime1 = millis();
  }

  if ((millis() - lastDebounceTime1) > debounceDelay) {
    // If the button state has changed:
    if (reading1 != buttonState1) {
      buttonState1 = reading1;
      // If the new state is LOW, the button was pressed
      if (buttonState1 == LOW) {
        buttonPressCount1++;
        if (buttonPressCount1 % 2 == 0) {
          sendLoRaCommand("R1_ON");
        } else {
          sendLoRaCommand("R1_OFF");
        }
      }
    }
  }
  lastButtonState1 = reading1;

  // Read the state of the push button 2
  int reading2 = digitalRead(buttonPin2);

  // If the switch changed, due to noise or pressing:
  if (reading2 != lastButtonState2) {
    // reset the debouncing timer
    lastDebounceTime2 = millis();
  }

  if ((millis() - lastDebounceTime2) > debounceDelay) {
    // If the button state has changed:
    if (reading2 != buttonState2) {
      buttonState2 = reading2;
      // If the new state is LOW, the button was pressed
      if (buttonState2 == LOW) {
        buttonPressCount2++;
        if (buttonPressCount2 % 2 == 0) {
          sendLoRaCommand("R2_ON");
        } else {
          sendLoRaCommand("R2_OFF");
        }
      }
    }
  }
  lastButtonState2 = reading2;
}

BLYNK_WRITE(V1) {
  if (param.asInt() == 1) {
    Serial.println("Button 1 pressed");
    sendLoRaCommand("R1_ON");
  } else {
    sendLoRaCommand("R1_OFF");
  }
}

BLYNK_WRITE(V2) {
  if (param.asInt() == 1) {
    Serial.println("Button 2 pressed");
    sendLoRaCommand("R2_ON");
  } else if (param.asInt() == 0) {
    Serial.println("Button 2 pressed");
    sendLoRaCommand("R2_OFF");
  }
}

void sendLoRaCommand(String command) {
  LoRa.beginPacket();
  LoRa.print(command);
  LoRa.endPacket();
  Serial.println("Sent LoRa command: " + command);
}
