#include <ESP32Servo.h> // Servo motor library
#include "Common.h"

// RFID-RC522
#include <MFRC522.h>

const int pinPotentiometer = 34;

const int pinFrontMotor = 13;
Servo servoFront; // Servo motor object instance

const int pinBackMotor = 5;
Servo servoBack; // Servo motor object instance

const int pinInternalLed = 2;

const int pinRedFrontLed = 33;
const int pinGreenFrontLed = 25;

const int pinRedBackLed = 14;
const int pinGreenBackLed = 12;

#define SS_PIN    21
#define RST_PIN   22
#define SIZE_BUFFER     18
#define MAX_SIZE_BLOCK  16
MFRC522::MIFARE_Key key; // MIFARE key object instance
MFRC522::StatusCode status; // Authentication status code
MFRC522 mfrc522(SS_PIN, RST_PIN);

void initSensors(){
  Serial.begin(115200);
  SPI.begin(); // Initialize SPI bus

  servoFront.attach(pinFrontMotor); // Connect Servo object instance to digital output 13
  servoFront.write(90); // Set the initial position of the servo

  servoBack.attach(pinBackMotor); // Connect Servo object instance to digital output
  servoBack.write(90); // Set the initial position of the servo

  pinMode(pinRedFrontLed, OUTPUT); // Define front red LED pin mode as output
  pinMode(pinGreenFrontLed, OUTPUT); // Define front green LED pin mode as output

  pinMode(pinRedBackLed, OUTPUT); // Define back red LED pin mode as output
  pinMode(pinGreenBackLed, OUTPUT); // Define back green LED pin mode as output

  pinMode(pinInternalLed, OUTPUT); // Define internal LED pin as output

  mfrc522.PCD_Init(); // Initialize the MFRC522 RFID module
}