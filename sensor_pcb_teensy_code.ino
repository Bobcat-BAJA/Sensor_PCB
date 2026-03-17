#include <FlexCAN_T4.h>

// --- Pin Definitions ---
const int wheelSpeedSensor1_Pin = 14; // Digital input (Wheel Speed 1)
const int wheelSpeedSensor2_Pin = 15; // Digital input (Wheel Speed 2)
const int shockTravelSensor1_Pin = 16; // Analog input (Shock Travel 1)
const int shockTravelSensor2_Pin = 17; // Analog input (Shock Travel 2)

// Variables, volatile is required for variables modified inside an interrupt routine
volatile unsigned long wheelSpeed1_Pulses = 0;
volatile unsigned long wheelSpeed2_Pulses = 0;

int shockTravel1_Val = 0;
int shockTravel2_Val = 0;

// Initialize CAN1 (Pins 0 RX, 1 TX), not really sure if this is the right way to do CAN
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;

// Interrupt Service Routines (ISRs), run automatically in the background every time a pulse is detected, for the high/low readings
void countPulse1() {
  wheelSpeed1_Pulses++;
}
void countPulse2() {
  wheelSpeed2_Pulses++;
}
void setup() {
  Serial.begin(115200);

  // INPUT_PULLUP prevents from reading random noise.
  pinMode(wheelSpeedSensor1_Pin, INPUT_PULLUP);
  pinMode(wheelSpeedSensor2_Pin, INPUT_PULLUP);
  
  // Attach interrupts to trigger on the falling edge of the pulse
  attachInterrupt(digitalPinToInterrupt(wheelSpeedSensor1_Pin), countPulse1, FALLING);
  attachInterrupt(digitalPinToInterrupt(wheelSpeedSensor2_Pin), countPulse2, FALLING);

  // Set analog pins as inputs
  pinMode(shockTravelSensor1_Pin, INPUT);
  pinMode(shockTravelSensor2_Pin, INPUT);

  // Setup CAN Bus, not really sure if this is the correct way to do can
  can1.begin();
  can1.setBaudRate(500000); // 500kbps is standard automotive speed
  Serial.println("CAN1 Initialized at 500kbps");
}

void loop() {
  // Read Analog Shock Sensors (Returns a value from 0 to 1023 because of 12-bit onboard ADC in Teensy)
  shockTravel1_Val = analogRead(shockTravelSensor1_Pin);
  shockTravel2_Val = analogRead(shockTravelSensor2_Pin);

  // Safely grab the interrupt counters
  noInterrupts();
  unsigned long currentPulses1 = wheelSpeed1_Pulses;
  unsigned long currentPulses2 = wheelSpeed2_Pulses;
  interrupts();

  // Print the data to the Serial Monitor to verify board operation
  Serial.print("Shock 1 (Pin 16): ");
  Serial.print(shockTravel1_Val);
  Serial.print("\tShock 2 (Pin 17): ");
  Serial.print(shockTravel2_Val);
  
  Serial.print("\tWS 1 Pulses (Pin 14): ");
  Serial.print(currentPulses1);
  Serial.print("\tWS 2 Pulses (Pin 15): ");
  Serial.println(currentPulses2);

  delay(100); // Wait 100 milliseconds before reading again, is this frequent enough?
}