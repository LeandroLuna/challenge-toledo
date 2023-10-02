#ifndef TAGS_H
#define TAGS_H

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

void checkCardData(){
  if (mfrc522.PICC_IsNewCardPresent()) { // Wait for the card to approach
    if (mfrc522.PICC_ReadCardSerial()) { // Select one of the cards
      tagData = readData(); // Read the card data
      tagData.trim(); // Remove empty spaces
      if(toBreakCellCounter < 3){
        toBreakCellCounter += 1;
      }
      mfrc522.PICC_HaltA(); // Instruct the PICC when in ACTIVE state to go into a "halt" state
      mfrc522.PCD_StopCrypto1();  // Stop the encryption of the PCD, should be called after communication with authentication, otherwise, new communications cannot be initiated
    }
  }
}

#endif