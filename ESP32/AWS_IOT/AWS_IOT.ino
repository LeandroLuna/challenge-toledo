// Geral
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Sensors
#include <NewPing.h> // Ultrassonic Sensor
#include <ESP32Servo.h> // Lib motor 

// Display
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// MQTT + AWS
#include "Secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
 
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
 
const int pinPotentiometer = 34;

const int pinMotor = 13;
Servo servo; // Obj Servo instance

const int pinButton = 32;

const int pinRedLed = 33;
const int pinGreenLed = 25;

#define TRIGGER_PIN 26
#define ECHO_PIN 27
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

// Main Func Variables
unsigned long frontGateTimer = 0; // Timer for front gate
unsigned long backGateTimer = 0;  // Timer for back gate
bool isFrontGateOpen = false;
bool isBackGateOpen = false;
bool publishedAfterFrontGate = false;

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}
 
void publishMessage(int potentiometer, double truckId)
{
  StaticJsonDocument<200> doc;
  doc["potentiometer"] = potentiometer;
  doc["truck_id"] = truckId;
  // doc["distance"] = distance;
  // doc["isGateOpen"] = gateStatus;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
 
void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}
 
void setup()
{
  Serial.begin(115200);
  srand(time(NULL));

  servo.attach(pinMotor); // Connect Servo obj instante to digital output 13
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

  connectAWS();
  delay(2000); // Pause for 2 seconds
}

void loop()
{
  int potentiometerValue = analogRead(pinPotentiometer);
  int isButtonPressed = digitalRead(pinButton);
  int distanceValue;
  distanceValue = sonar.ping_cm();
  unsigned long currentTime = millis();

  if(digitalRead(pinButton) == LOW && !isFrontGateOpen){ // Button was pressed
    isFrontGateOpen = true;
    frontGateTimer = currentTime;
    publishedAfterFrontGate = false;
  } 

  if (isFrontGateOpen && !isBackGateOpen && (currentTime - frontGateTimer) >= 10000) {
    isBackGateOpen = true;
    backGateTimer = currentTime;
    isFrontGateOpen = false;
  } else if (isBackGateOpen && (currentTime - backGateTimer) >= 10000) {
    isBackGateOpen = false; // Reset isBackGateOpen after 10 seconds
    publishedAfterFrontGate = false;
  }

  if (isBackGateOpen && !publishedAfterFrontGate) {
    double truckIdentifier = generateRandomNumber(1, 2);
    publishMessage(potentiometerValue, truckIdentifier);
    publishedAfterFrontGate = true;
  }

  if(isFrontGateOpen){
    servo.write(90); // Open gate
    digitalWrite(pinGreenLed, HIGH); // Green ON
    digitalWrite(pinRedLed, LOW);
  } else {
    servo.write(0);
    digitalWrite(pinRedLed, HIGH); // Red ON
    digitalWrite(pinGreenLed, LOW);
  }

  sendSerialData(potentiometerValue, isFrontGateOpen, isBackGateOpen);

  setDisplayText(potentiometerValue, distanceValue, isFrontGateOpen, isBackGateOpen);
  client.loop();
}


void setDisplayText(int potentiometer, int distance, bool frontGateStatus, bool backGateStatus) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 4);
  display.print("Potentiometer: " + String(potentiometer)); 
  
  display.setCursor(0, 14);
  display.print("Distance: " + String(distance) + "cm"); 
  
  display.setCursor(0, 24);
  if(frontGateStatus){
    display.print("F.G. state: open"); 
  } else if (backGateStatus){
    display.print("B.G. state: open"); 
  } else {
    display.print("Both gates closed"); 
  }

  display.display();
}

void sendSerialData(int potentiometer, bool frontGate, bool backGate){
  Serial.print(potentiometer);
  Serial.print(" ");
  Serial.print(frontGate);
  Serial.print(" ");
  Serial.print(backGate);
  Serial.println(); // Move to next line to print out new values
  Serial.flush(); // Clear buffer
}

int generateRandomNumber(int minValue, int maxValue) {
  return rand() % (maxValue - minValue + 1) + minValue;
}