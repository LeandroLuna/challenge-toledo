// General
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SPI.h>
#include <cmath> // Lib cmath for sqrt calc

// Sensors
#include <NewPing.h> // Ultrassonic Sensor
#include <ESP32Servo.h> // Lib motor 

// RFID-RC522
#include <MFRC522.h> 

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

const int pinFrontMotor = 13;
Servo servoFront; // Obj Servo instance

const int pinBackMotor = 5;
Servo servoBack; // Obj Servo instance

const int pinInternalLed = 2;

const int pinRedFrontLed = 33;
const int pinGreenFrontLed = 25;

const int pinRedBackLed = 14;
const int pinGreenBackLed = 12;

#define TRIGGER_PIN 26
#define ECHO_PIN 27
#define MAX_DISTANCE 400
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // Obj sonar (ultrassonic) instance

#define SS_PIN    21
#define RST_PIN   22
#define SIZE_BUFFER     18
#define MAX_SIZE_BLOCK  16
MFRC522::MIFARE_Key key; // Obj MIFARE_Key instance
MFRC522::StatusCode status; // Auth status code
MFRC522 mfrc522(SS_PIN, RST_PIN); 

// Main Func Variables
unsigned long frontGateTimer = 0; // Timer for front gate
unsigned long backGateTimer = 0;  // Timer for back gate
bool isFrontGateOpen = false;
bool isBackGateOpen = false;
bool publishedAfterFrontGate = false;
String tagData = "";
int distanceValue;
int potentiometerValue;
unsigned long currentTime;
bool isSafeDistance;
String receivedDataSerial = ""; // Variable to store the received data from Serial
int totalWeight;
int distributedWeight;
int isCellBroken = 0;
int weightCell[6];
int brokeCellIndex;
double stdDeviation;
int toBreakCellCounter = 0;
int isCellValueSetted = 0;
int removedWeight;

void connectAWS(){
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
 
  client.setServer(AWS_IOT_ENDPOINT, 8883); // Connect to the MQTT broker on the AWS endpoint we defined earlier
 
  client.setCallback(messageHandler); // Create a message handler
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME)){
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC); // Subscribe to a topic
 
  Serial.println("AWS IoT Connected!");
}
 
void publishMessage(String truck, int simulatedTotalWeight, int simulatedWeightCell[], double stdDeviation){
  int truckId;
    
  if(truck.equals("Truck1")){
    truckId = 1;
  } else if (truck.equals("Truck2")){
    truckId = 2;
  }

  StaticJsonDocument<512> doc;
  doc["truck_id"] = truckId;
  for (int i = 0; i < 6; i++) {
    doc["weight_cell_" + String(i + 1)] = simulatedWeightCell[i];
  }
  doc["standard_deviation"] = int(stdDeviation);
  doc["total_weight"] = simulatedTotalWeight;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // Print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
 
// RECEIVE PUBLISHED MESSAGES IN ESP32/SUB TOPIC  
void messageHandler(char* topic, byte* payload, unsigned int length){
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload);
  
  // if (doc.containsKey("cellPairs")) { // Check if the "cellPairs" field is present in the JSON
  //   JsonArray cellPairs = doc["cellPairs"].as<JsonArray>(); // Get the JSON array of objects "cellPairs"

  //   for (JsonObject cellPair : cellPairs) { // Iterate through the objects in the "cellPairs" array
  //     int cellPairId = cellPair["cellPairId"];
  //     const char* status = cellPair["status"];
      
  //     Serial.print("Cell Pair ID: ");
  //     Serial.println(cellPairId);
  //     Serial.print("Status: ");
  //     Serial.println(status);
  //   }
  // }
  return;
}
 
void setup(){
  Serial.begin(115200);
  SPI.begin(); // Init SPI bus
  srand(time(NULL));

  servoFront.attach(pinFrontMotor); // Connect Servo obj instante to digital output 13
  servoFront.write(0); // Initial position

  servoBack.attach(pinBackMotor); // Connect Servo obj instante to digital output 
  servoBack.write(0); // Initial position

  pinMode(pinRedFrontLed, OUTPUT); // Define front red led pin mode as output
  pinMode(pinGreenFrontLed, OUTPUT); // Define front green led pin mode as output

  pinMode(pinRedBackLed, OUTPUT); // Define back red led pin mode as output
  pinMode(pinGreenBackLed, OUTPUT); // Define back green led pin mode as output

  pinMode(pinInternalLed, OUTPUT); // Define internal led pin

  mfrc522.PCD_Init(); // Init MFRC522

  connectAWS();
  delay(2000); // Pause for 2 seconds
}

void loop(){
  potentiometerValue = analogRead(pinPotentiometer);
  distanceValue = sonar.ping_cm();
  currentTime = millis();

  totalWeight = potentiometerValue * 10;
  distributedWeight = totalWeight / 6;

  for (int i = 0; i < 6; i++) {
    weightCell[i] = distributedWeight;
  }

  while (Serial.available() > 0) { // Check if there's data available for reading
    char receivedChar = Serial.read(); // Read a character from the serial port
    
    if (receivedChar == '\n') { // Check if it's the newline character (end of string)
      if((receivedDataSerial.toInt() == brokeCellIndex) && isCellBroken){ // Fix cell
        isCellBroken = 0;
        isCellValueSetted = 0;
        toBreakCellCounter = 0;
      }
      if(receivedDataSerial.toInt() == 99){ // Write new data to tags
        writeData();
      }
      receivedDataSerial = ""; // Clear the variable for the next string
    } else {
      receivedDataSerial += receivedChar; // Accumulate the characters in the receivedDataSerial variable
    }
  }

  if (mfrc522.PICC_IsNewCardPresent()) { // Wait for the card to approach
    if (mfrc522.PICC_ReadCardSerial()) { // Select one of the cards
      tagData = readData(); // Read the card data
      tagData.trim(); // Remove empty spaces
      if(toBreakCellCounter <= 3){
        toBreakCellCounter += 1;
      }
      mfrc522.PICC_HaltA(); // Instruct the PICC when in ACTIVE state to go into a "halt" state
      mfrc522.PCD_StopCrypto1();  // Stop the encryption of the PCD, should be called after communication with authentication, otherwise, new communications cannot be initiated
    }
  }

  if(toBreakCellCounter == 3){
    isCellBroken = 1;
  }

  if(isCellBroken == 1 && isCellValueSetted == 0){
    brokeCellIndex = int(generateRandomNumber(0, 5));
    isCellValueSetted = 1;
  }

  if(isCellBroken == 1){
    removedWeight = int(generateRandomNumber(100, 1000));
    if((weightCell[brokeCellIndex] - removedWeight) >= 0){
      weightCell[brokeCellIndex] = weightCell[brokeCellIndex] - removedWeight;
    } else {
      removedWeight = weightCell[brokeCellIndex];
      weightCell[brokeCellIndex] = 0;
    }
    totalWeight -= removedWeight;
  }

  stdDeviation = calculateStandardDeviation(weightCell);

  if(tagData.length() > 0 && !isFrontGateOpen && isSafeDistance){ // Start weighing
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
    publishMessage(tagData, totalWeight, weightCell, stdDeviation);
    publishedAfterFrontGate = true;
    tagData.clear();
  }

  if(isFrontGateOpen){
    servoFront.write(90); // Open front gate
    digitalWrite(pinGreenFrontLed, HIGH); // Green ON
    digitalWrite(pinRedFrontLed, LOW);
  } else {
    servoFront.write(0);
    digitalWrite(pinRedFrontLed, HIGH); // Red ON
    digitalWrite(pinGreenFrontLed, LOW);
  }

  if(isBackGateOpen){
    servoBack.write(90); // Open back gate
    digitalWrite(pinGreenBackLed, HIGH); // Green ON
    digitalWrite(pinRedBackLed, LOW);
  } else {
    servoBack.write(0);
    digitalWrite(pinRedBackLed, HIGH); // Red ON
    digitalWrite(pinGreenBackLed, LOW);
  }

  if(distanceValue < 10){
    digitalWrite(pinInternalLed, HIGH);
    isSafeDistance = false;
  } else {
    digitalWrite(pinInternalLed, LOW);
    isSafeDistance = true;
  }

  sendSerialData(tagData, totalWeight, weightCell, isFrontGateOpen, isBackGateOpen);

  client.loop();
}

void sendSerialData(String whoRequestedIt, int simulatedTotalWeight, int simulatedWeightCell[], bool frontGate, bool backGate){
  Serial.print(whoRequestedIt.length() > 0 ? whoRequestedIt : "NoTruck");
  Serial.print(" ");
  Serial.print(simulatedTotalWeight);
  Serial.print(" ");
  for (int i = 0; i < 6; i++) {
    Serial.print(simulatedWeightCell[i]);
    Serial.print(" ");
  }
  Serial.print(frontGate);
  Serial.print(" ");
  Serial.print(backGate);
  Serial.print(" ");
  Serial.println(); // Move to next line to print out new values
  Serial.flush(); // Clear buffer
}

int generateRandomNumber(int minValue, int maxValue) {
  return rand() % (maxValue - minValue + 1) + minValue;
}

double calculateStandardDeviation(int cellData[]) {
  double mean = 0.0;
  double sum = 0.0;
  int length = 6;
  
  // Average
  for (int i = 0; i < length; i++) {
    mean += cellData[i];
  }
  mean /= length;

  // Squares sum
  for (int i = 0; i < length; i++) {
    double diff = cellData[i] - mean;
    sum += diff * diff;
  }

  double variance = sum / length; // Variance

  double standardDeviation = sqrt(variance); // Standard Deviation (Variance squared root)

  return standardDeviation;
}

String readData(){
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF; // Prepare the key - all keys are set to FFFFFFFFFFFFh (Factory default).

  byte buffer[SIZE_BUFFER] = {0}; // Buffer to store the read data

  // Block we will operate on
  byte block = 1;
  byte size = SIZE_BUFFER;

  mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid)); // Authenticate the block we are going to operate on

  mfrc522.MIFARE_Read(block, buffer, &size); // Read data from the block

  uint8_t entireBuffer[MAX_SIZE_BLOCK];
  
  // Write read data to string
  String bufferString = "";
  for (uint8_t i = 0; i < MAX_SIZE_BLOCK; i++){
    bufferString += char(buffer[i]);
  }
  
  return bufferString;
}

// Write data to the card/tag 
void writeData(){
  // Print the technical details of the card/tag
  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid));
  // Wait for 30 seconds for data input via Serial
  Serial.setTimeout(30000L);
  Serial.println(F("Enter the data to be written with the '#' character at the end\n[maximum of 16 characters]:"));

  // Prepare the key - all keys are set to FFFFFFFFFFFFh (Factory default).
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  // Buffer to store the data to be written
  byte buffer[MAX_SIZE_BLOCK] = "";
  byte block; // Block on which we wish to perform the operation
  byte dataSize; // Size of the data we will operate on (in bytes)

  // Retrieve data entered by the user via Serial
  // It will be all data before the '#' character
  dataSize = Serial.readBytesUntil('#', (char*)buffer, MAX_SIZE_BLOCK);
  // Fill any remaining space in the buffer with spaces
  for (byte i = dataSize; i < MAX_SIZE_BLOCK; i++)
  {
    buffer[i] = ' ';
  }

  block = 1; // Block defined for the operation
  String str = (char*)buffer; // Convert the data to a string for printing
  Serial.println(str);

  // Authenticate is a command for authentication to enable secure communication
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
    block, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  // else Serial.println(F("PCD_Authenticate() success: "));

  // Write to the block
  status = mfrc522.MIFARE_Write(block, buffer, MAX_SIZE_BLOCK);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
}