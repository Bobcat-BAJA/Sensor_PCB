// Defining pins
const int wheelSpeedRear_Pin = 14;     
const int wheelSpeedFront_Pin = 15;    
const int rightShockFront_Pin = 16;    
const int leftShockFront_Pin = 17;     

// Variables
volatile unsigned long wheelSpeedRear_Pulses = 0;
volatile unsigned long wheelSpeedFront_Pulses = 0;

int rightShockFront_Val = 0;
int leftShockFront_Val = 0;

// Interrupt Service Routines
void countPulseRear() {
  wheelSpeedRear_Pulses++;
}
void countPulseFront() {
  wheelSpeedFront_Pulses++;
}

void setup() {
  Serial.begin(115200);

  // Wait briefly for the Serial Monitor to open
  delay(2000); 
  Serial.println("Starting Sensor Test...");

  // Wheel Speed Inputs
  pinMode(wheelSpeedRear_Pin, INPUT_PULLUP);
  pinMode(wheelSpeedFront_Pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(wheelSpeedRear_Pin), countPulseRear, FALLING);
  attachInterrupt(digitalPinToInterrupt(wheelSpeedFront_Pin), countPulseFront, FALLING);

  // Shock Inputs
  pinMode(rightShockFront_Pin, INPUT);
  pinMode(leftShockFront_Pin, INPUT);

  // Set Teensy ADC to 12-bit (0-4095)
  analogReadResolution(12);
}

void loop() {
  // Read the raw 12-bit analog value directly (Returns 0 to 4095)
  rightShockFront_Val = analogRead(rightShockFront_Pin);
  leftShockFront_Val = analogRead(leftShockFront_Pin);

  // Get interrupt counters safely for speed
  noInterrupts();
  unsigned long currentPulsesRear = wheelSpeedRear_Pulses;
  unsigned long currentPulsesFront = wheelSpeedFront_Pulses;
  
  // Reset counters for speed calculation
  wheelSpeedRear_Pulses = 0; 
  wheelSpeedFront_Pulses = 0;
  interrupts();

  // ==========================================
  // PRINTING TO SERIAL PLOTTER
  // ==========================================
  
  Serial.print("RightShock:");
  Serial.print(rightShockFront_Val);
  Serial.print(",");

  Serial.print("LeftShock:");
  Serial.print(leftShockFront_Val);
  Serial.print(",");

  Serial.print("RearSpeedPulses:");
  Serial.print(currentPulsesRear);
  Serial.print(",");

  Serial.print("FrontSpeedPulses:");
  Serial.println(currentPulsesFront); 
    
  // Running at 10 Hz for testing
  delay(100);
}