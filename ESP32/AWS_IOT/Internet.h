#include "Secrets.h"                // Include secrets (like Wi-Fi credentials)
#include <WiFiClientSecure.h>       // Include the Wi-Fi Client Secure library
#include <PubSubClient.h>           // Include the MQTT PubSubClient library
#include <ArduinoJson.h>            // Include the Arduino JSON library
#include "WiFi.h"                   // Include the ESP32 Wi-Fi library

#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"    // Define the MQTT publish topic
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"    // Define the MQTT subscribe topic

WiFiClientSecure net = WiFiClientSecure();    // Create a secure Wi-Fi client
PubSubClient client(net);                     // Create an MQTT PubSubClient

// Function to handle incoming MQTT messages
void messageHandler(char* topic, byte* payload, unsigned int length){
  StaticJsonDocument<200> doc;                 // Create a JSON document to hold the received data
  DeserializationError error = deserializeJson(doc, payload); // Deserialize the received JSON

  // Uncomment and modify the following code to process received JSON data
  // if (doc.containsKey("cellPairs")) {
  //   JsonArray cellPairs = doc["cellPairs"].as<JsonArray>();

  //   for (JsonObject cellPair : cellPairs) {
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

// Function to connect to AWS IoT
void connectAWS(){
  WiFi.mode(WIFI_STA);                       // Set Wi-Fi mode to station
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);      // Connect to Wi-Fi using provided credentials

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  client.setServer(AWS_IOT_ENDPOINT, 8883); // Connect to the MQTT broker on the AWS endpoint

  client.setCallback(messageHandler); // Create a message handler

  Serial.println("Connecting to AWS IoT");

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

// Function to publish a message to AWS IoT
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
  serializeJson(doc, jsonBuffer); // Serialize the JSON data

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer); // Publish the JSON data to the AWS IoT topic
}
