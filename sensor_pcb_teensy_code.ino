#include <FlexCAN_T4.h>

// Defining pins
const int wheelSpeedRear_Pin = 14;     // Digital input (Wheel Speed 1)
const int wheelSpeedFront_Pin = 15;    // Digital input (Wheel Speed 2)
const int rightShockFront_Pin = 16;    // Analog input (Shock Travel 1)
const int leftShockFront_Pin = 17;     // Analog input (Shock Travel 2)

// Variables
volatile unsigned long wheelSpeedRear_Pulses = 0;
volatile unsigned long wheelSpeedFront_Pulses = 0;

// Multi-turn tracking variables
long rightShockFront_Total = 0;
int rightShockFront_Prev = 0;
int rightShockFront_Rotations = 0;

long leftShockFront_Total = 0;
int leftShockFront_Prev = 0;
int leftShockFront_Rotations = 0;

// Initialize CAN1
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
CAN_message_t msg; 

// Interrupt Service Routines
void countPulseRear() {
  wheelSpeedRear_Pulses++;
}
void countPulseFront() {
  wheelSpeedFront_Pulses++;
}

void setup() {
  Serial.begin(115200);

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

  can1.begin();
  can1.setBaudRate(500000); 
  
  msgFront.id = 0x08; // GPIO Front
  msgRear.id = 0x09;  // GPIO REAR
  msg.len = 8; 

  // Grab the initial shock values
  rightShockFront_Prev = analogRead(rightShockFront_Pin);
  leftShockFront_Prev = analogRead(leftShockFront_Pin);
}

// Helper function to calculate continuous multi-turn travel
long getMultiTurnShock(int pin, int &prevVal, int &rotations) {
  int currentVal = analogRead(pin);
  
  if (currentVal - prevVal < -2048) {
    rotations++; 
  } else if (currentVal - prevVal > 2048) {
    rotations--; 
  }
  
  prevVal = currentVal;
  return (rotations * 4096) + currentVal; 
}

void loop() {
  // Update the continuous multi-turn shock values
  rightShockFront_Total = getMultiTurnShock(rightShockFront_Pin, rightShockFront_Prev, rightShockFront_Rotations);
  leftShockFront_Total = getMultiTurnShock(leftShockFront_Pin, leftShockFront_Prev, leftShockFront_Rotations);

  // Get interrupt counters safely for speed
  noInterrupts();
  unsigned long currentPulsesRear = wheelSpeedRear_Pulses;
  unsigned long currentPulsesFront = wheelSpeedFront_Pulses;
  
  // Reset for speed calculation (pulses per 100ms)
  wheelSpeedRear_Pulses = 0; 
  wheelSpeedFront_Pulses = 0;
  interrupts();

  // ==========================================
  // PACKING THE CAN MESSAGE 
  // ==========================================
  
  // Bytes 0 and 1: Right Shock Front
  uint16_t rsf_16bit = (uint16_t)rightShockFront_Total;
  msg.buf[0] = (rsf_16bit >> 8) & 0xFF;
  msg.buf[1] = rsf_16bit & 0xFF;
  
  // Bytes 2 and 3: Left Shock Front
  uint16_t lsf_16bit = (uint16_t)leftShockFront_Total;
  msg.buf[2] = (lsf_16bit >> 8) & 0xFF;
  msg.buf[3] = lsf_16bit & 0xFF;

  // Bytes 4 and 5: Wheel Speed Rear
  uint16_t wsr_16bit = (uint16_t)currentPulsesRear;
  msg.buf[4] = (wsr_16bit >> 8) & 0xFF;
  msg.buf[5] = wsr_16bit & 0xFF;
  
  // Bytes 6 and 7: Wheel Speed Front (Occupying the unused shock address in the CAN code)
  uint16_t wsf_16bit = (uint16_t)currentPulsesFront;
  msg.buf[6] = (wsf_16bit >> 8) & 0xFF;
  msg.buf[7] = wsf_16bit & 0xFF;

  // Send the message
  can1.write(msg);
    
  delay(100);
}
