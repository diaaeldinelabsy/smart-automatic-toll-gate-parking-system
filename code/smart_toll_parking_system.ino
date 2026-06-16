/*
Smart Automatic Toll Gate & Car Parking System Using Arduino & RFID

Authors: DiaaEldin Elabsy & Suchi Chowdhury

Description: Arduino-based smart parking and toll gate system using RFID,
IR sensors, servo motor, OLED display, LEDs, and buzzer for
automatic vehicle access control and parking management.
*/

#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED 

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// RFID

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);

// IR Sensors 

#define IR_ENTRY 2
#define IR_EXIT 3

// Traffic Light

#define GREEN_LED 4
#define YELLOW_LED 7
#define RED_LED 8

// Buzzer

#define BUZZER 5

// Servo

#define SERVO_PIN 6

Servo gateServo;

const int GATE_OPEN = 90;
const int GATE_CLOSE = 0;

// Parking

int totalSpots = 3;
int filledSpots = 0;

// RFID TAGS (REAL UIDs)

byte Tag1[4] = {0xD1, 0xBD, 0xD5, 0x21};
byte Tag2[4] = {0xD3, 0xEB, 0xEF, 0x00};

// SYSTEM STATE

bool entryActive = false;
bool exitActive = false;

// SETUP

void setup() {

  Serial.begin(9600);

  SPI.begin();
  rfid.PCD_Init();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  pinMode(IR_ENTRY, INPUT);
  pinMode(IR_EXIT, INPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  gateServo.attach(SERVO_PIN);
  gateServo.write(GATE_CLOSE);

  trafficRed();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(10, 10);
  display.println("SMART PARKING");

  display.setCursor(10, 30);
  display.println("SYSTEM READY");

  display.display();

  delay(2500);

  updateOLED();
}


// LOOP

void loop() {

  if (!entryActive && digitalRead(IR_ENTRY) == LOW) {

    entryActive = true;

    trafficYellow();

    display.clearDisplay();

    display.setCursor(10, 5);
    display.println("ENTRY DETECTED");

    display.setCursor(0, 25);
    display.print("Filled: ");
    display.println(filledSpots);

    display.setCursor(0, 40);
    display.print("Empty: ");
    display.println(totalSpots - filledSpots);

    display.setCursor(0, 55);
    display.println("SCAN RFID");

    display.display();

    while (true) {

      if (rfid.PICC_IsNewCardPresent() &&
          rfid.PICC_ReadCardSerial()) {

        if (checkRFID()) {

          break;

        } else {

          display.clearDisplay();

          display.setCursor(10, 25);
          display.println("ACCESS DENIED");
          display.display();

          delay(1200);

          display.clearDisplay();

          display.setCursor(0, 55);
          display.println("SCAN RFID");
          display.display();
        }
      }

      delay(50);
    }

    display.clearDisplay();

    display.setCursor(20, 20);
    display.println("ACCESS GRANTED");
    display.display();

    delay(1000);

    gateOpen();

    while (digitalRead(IR_EXIT) == LOW) {
      delay(50);
    }

    if (filledSpots < totalSpots)
      filledSpots++;

    updateOLED();

    while (digitalRead(IR_EXIT) == HIGH) {
      delay(50);
    }

    gateClose();

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    entryActive = false;
  }

  if (!exitActive && digitalRead(IR_EXIT) == LOW) {

    exitActive = true;

    display.clearDisplay();

    display.setCursor(10, 10);
    display.println("EXIT DETECTED");

    display.setCursor(10, 30);
    display.println("OPENING GATE");

    display.display();

    trafficGreen();
    gateOpen();

    while (digitalRead(IR_ENTRY) == HIGH) {
      delay(50);
    }

    while (digitalRead(IR_ENTRY) == LOW) {
      delay(50);
    }

    gateClose();

    if (filledSpots > 0)
      filledSpots--;

    updateOLED();

    exitActive = false;
  }

  trafficRed();
}


// OLED UPDATE


void updateOLED() {

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("PARKING STATUS");

  display.setCursor(0, 20);
  display.print("Filled: ");
  display.println(filledSpots);

  display.setCursor(0, 35);
  display.print("Empty: ");
  display.println(totalSpots - filledSpots);

  display.display();
}


// RFID CHECK


bool checkRFID() {

  if (uidMatch(Tag1)) return true;
  if (uidMatch(Tag2)) return true;

  return false;
}

bool uidMatch(byte *tag) {

  for (byte i = 0; i < 4; i++) {

    if (rfid.uid.uidByte[i] != tag[i])
      return false;
  }

  return true;
}

// GATE CONTROL

void gateOpen() {

  gateServo.write(GATE_OPEN);

  trafficGreen();

  tone(BUZZER, 1000);
  delay(300);
  noTone(BUZZER);
}

void gateClose() {

  gateServo.write(GATE_CLOSE);

  trafficRed();

  tone(BUZZER, 500);
  delay(300);
  noTone(BUZZER);
}


// TRAFFIC LIGHTS

void trafficRed() {

  digitalWrite(RED_LED, HIGH);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
}

void trafficYellow() {

  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, HIGH);
  digitalWrite(GREEN_LED, LOW);
}

void trafficGreen() {

  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
}
