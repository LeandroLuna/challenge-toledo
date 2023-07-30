#include <Servo.h> // Lib motor 
#include <NewPing.h> // Ultrassonic Sensor

// Display
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const int pinMotor = 3;
Servo servo; // Obj Servo instance

const int pinButton = 2;

const int pinRedLed = 4;
const int pinGreenLed = 5;

#define TRIGGER_PIN 12
#define ECHO_PIN 11
#define MAX_DISTANCE 400
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // Obj sonar (ultrassonic) instance

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600); // Start serial port
  servo.attach(pinMotor); // Connect Servo obj instante to D3 
  servo.write(0); // Initial position
  pinMode(pinButton, INPUT_PULLUP); // Define button pin read mode using internal Arduino resistor

  pinMode(pinRedLed, OUTPUT); // Define red led pin mode as output
  pinMode(pinGreenLed, OUTPUT); // Define green led pin mode as output

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Display allocation failed");
    for(;;); // Don't proceed, loop forever
  }

  // display.display();
  display.clearDisplay();
  delay(2000); // Pause for 2 seconds
}

void loop() {
  int potentiometerValue = analogRead(A0); // Read potentiometer value. 0-750 using 3.3V
  int buttonPressed = digitalRead(pinButton);
  bool isGateOpen = false;

  if(buttonPressed == LOW){ // Button was pressed
    servo.write(90); // Open gate
    digitalWrite(pinGreenLed, HIGH); // Green ON
    digitalWrite(pinRedLed, LOW);
    isGateOpen = true;
  } else {
    servo.write(0);
    digitalWrite(pinRedLed, HIGH); // Red ON
    digitalWrite(pinGreenLed, LOW);
  }

  Serial.print(potentiometerValue);
  Serial.print(" ");
  Serial.print(isGateOpen);
  Serial.println(); // Move to next line to print out new values
  Serial.flush(); // Clear buffer

  int distanceValue;
  distanceValue = sonar.ping_cm();

  setDisplayText(potentiometerValue, distanceValue, isGateOpen);
	isGateOpen ? delay(10000) : delay(50);
}

void setDisplayText(int potentiometer, int distance, bool gateStatus) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 4);
  display.print("Potentiometer: " + String(potentiometer)); 
  
  display.setCursor(0, 14);
  display.print("Distance: " + String(distance) + "cm"); 
  
  display.setCursor(0, 24);
  display.print(gateStatus ? "Gate state: open" : "Gate state: closed" ); 

  display.display();
}