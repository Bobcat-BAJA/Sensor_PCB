#include <LittleFS.h>

// Use Teensy's onboard program flash
LittleFS_Program myfs; 

void setup() {
  // Use a fast baud rate so the CSV dumps quickly
  Serial.begin(115200);
  
  // Pause the code here until you actually open the Serial Monitor
  while (!Serial) {
    ; 
  }

  // Initialize LittleFS with the exact same 1MB size used for logging
  if (!myfs.begin(1048576)) {
    Serial.println("Error starting LittleFS! Did you set the Tools -> Flash Size to 1MB?");
    return;
  }

  // Open the specific CSV file we created in your main code
  File dataFile = myfs.open("CAN_BACKUP.csv", FILE_READ);

  if (dataFile) {
    Serial.println("--- START OF DATA ---");
    
    // Read and print each character until the end of the file
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    
    dataFile.close();
    Serial.println("\n--- END OF DATA ---");
  } else {
    Serial.println("Error: Could not find CAN_BACKUP.csv. Was any data logged?");
  }
}

void loop() {
  // Empty. We only need to dump the data once when the board turns on.
}