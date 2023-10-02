#include "Common.h"
#include "Sensors.h"
#include "Internet.h"
#include "Utils.h"
#include "Tags.h"

void setup() {
  srand(time(NULL));  // Initialize random number generator based on the current time.
  initSensors();      // Initialize sensors and other hardware components.
  connectAWS();       // Connect to the AWS (Amazon Web Services) IoT platform.
  delay(2000);        // Wait for 2 seconds (2000 milliseconds).
}

void loop() {
  potentiometerValue = analogRead(pinPotentiometer);  // Read the analog value from a potentiometer.
  setCellsValue(potentiometerValue);                  // Set values for weight cells based on the potentiometer reading.
  readSerialIncomingData();                           // Read incoming data from a serial communication interface.
  checkCardData();                                    // Check for card data (assuming RFID or similar).
  processCellBreakage();                             // Process cell breakage logic.
  stdDeviation = calculateStandardDeviation(weightCell);  // Calculate the standard deviation of weight cell values.
  controlGateState();                                 // Control the state of gates (open or closed).
  controlInternalLed();                               // Control the state of an internal LED based on tag data.
  sendSerialData(tagData, totalWeight, weightCell, isFrontGateOpen, isBackGateOpen);  // Send data over a serial interface.
  client.loop();                                      // Handle AWS IoT client operations.
}