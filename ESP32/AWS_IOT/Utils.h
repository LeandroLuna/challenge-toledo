#include "Tags.h"

// Function to generate a random number within a specified range
int generateRandomNumber(int minValue, int maxValue) {
  return rand() % (maxValue - minValue + 1) + minValue;
}

// Function to read incoming data from the Serial communication
void readSerialIncomingData(){
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

      if(receivedDataSerial.toInt() == 200){
        isFrontGateOpen = !isFrontGateOpen;
        isBackGateOpen = false;
      }

      if(receivedDataSerial.toInt() == 201){
        isBackGateOpen = !isBackGateOpen;
        isFrontGateOpen = false;
      }

      receivedDataSerial = ""; // Clear the variable for the next string
    } else {
      receivedDataSerial += receivedChar; // Accumulate the characters in the receivedDataSerial variable
    }
  }
}

// Function to process cell breakage
void processCellBreakage(){
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
}

// Function to control gate states
void controlGateState(){
  if (isBackGateOpen && !publishedAfterFrontGate) {
    publishMessage(tagData, totalWeight, weightCell, stdDeviation);
    publishedAfterFrontGate = true;
    tagData.clear();
  }

  if(isFrontGateOpen && tagData.length() > 0){
    servoFront.write(180); // Open front gate
    digitalWrite(pinGreenFrontLed, HIGH); // Green LED ON
    digitalWrite(pinRedFrontLed, LOW);
    publishedAfterFrontGate = false;
  } else {
    servoFront.write(90);
    digitalWrite(pinRedFrontLed, HIGH); // Red LED ON
    digitalWrite(pinGreenFrontLed, LOW);
  }

  if(isBackGateOpen){
    servoBack.write(180); // Open back gate
    digitalWrite(pinGreenBackLed, HIGH); // Green LED ON
    digitalWrite(pinRedBackLed, LOW);
  } else {
    servoBack.write(90);
    digitalWrite(pinRedBackLed, HIGH); // Red LED ON
    digitalWrite(pinGreenBackLed, LOW);
  }
}

// Function to control internal LED based on tagData
void controlInternalLed(){
  if(tagData.length() > 0){
    digitalWrite(pinInternalLed, HIGH);
  } else {
    digitalWrite(pinInternalLed, LOW);
  }
}

// Function to send serial data
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
  Serial.println(); // Move to the next line to print out new values
  Serial.flush(); // Clear the buffer
}

// Function to calculate standard deviation of cell data
double calculateStandardDeviation(int cellData[]) {
  double mean = 0.0;
  double sum = 0.0;
  int length = 6;
  
  // Calculate the average
  for (int i = 0; i < length; i++) {
    mean += cellData[i];
  }
  mean /= length;

  // Calculate the sum of squared differences
  for (int i = 0; i < length; i++) {
    double diff = cellData[i] - mean;
    sum += diff * diff;
  }

  double variance = sum / length; // Calculate variance

  double standardDeviation = sqrt(variance); // Calculate standard deviation (square root of variance)

  return standardDeviation;
}

// Function to set values for the cells based on potentiometer value
void setCellsValue(int potentiometerValue){
  totalWeight = potentiometerValue * 10;
  distributedWeight = totalWeight / 6;

  for (int i = 0; i < 6; i++) {
    addVariation = generateRandomNumber(0, 200);
    weightCell[i] = distributedWeight + addVariation;
  }
}
