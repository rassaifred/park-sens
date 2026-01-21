#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <NewPing.h>

// --- Pin Definitions ---
#define OLED_RESET     -1 
#define SCREEN_ADDRESS 0x3C
#define TRIG_PIN       2
#define ECHO_PIN       3
#define MAX_DISTANCE   200
#define RED_LED        4
#define YELLOW_LED     5
#define GREEN_LED      6
#define BUTTON_PIN     7
#define RS485_RX       10
#define RS485_TX       11
#define RS485_CONTROL  9  // DE/RE Pins

// --- Object Initialization ---
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);
SoftwareSerial rs485(RS485_RX, RS485_TX);
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

void setup() {
  // Serial Monitor
  Serial.begin(9600);
  
  // RS485 Setup
  pinMode(RS485_CONTROL, OUTPUT);
  digitalWrite(RS485_CONTROL, LOW); // Set to Receive mode initially
  rs485.begin(9600);

  // Pin Modes
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // OLED Initialization
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void loop() {
  // 1. Read Sensors
  int distance = sonar.ping_cm();
  bool btnState = !digitalRead(BUTTON_PIN); // Active Low

  // 2. Logic: Traffic Light based on distance
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  if (distance > 0 && distance < 10) digitalWrite(RED_LED, HIGH);
  else if (distance >= 10 && distance < 20) digitalWrite(YELLOW_LED, HIGH);
  else digitalWrite(GREEN_LED, HIGH);

  // 3. Update OLED
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("SYSTEM READY");
  display.print("Dist: "); display.print(distance); display.println(" cm");
  display.print("Btn: "); display.println(btnState ? "PRESSED" : "OFF");
  display.display();

  // 4. RS485 Transmission (Send data every 2 seconds)
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 2000) {
    digitalWrite(RS485_CONTROL, HIGH); // Switch to Transmit
    rs485.print("Dist: ");
    rs485.println(distance);
    delay(10); // Wait for transmission to finish
    digitalWrite(RS485_CONTROL, LOW);  // Switch back to Receive
    lastSend = millis();
  }

  delay(100);
}