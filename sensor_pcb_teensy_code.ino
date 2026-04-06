#include <FlexCAN_T4.h>

// Defining pins
const int wheelSpeedRear_Pin = 14;     // Belongs to 0x09
const int wheelSpeedFront_Pin = 15;    // Belongs to 0x09 (Mapped to rightShockRear address)
const int rightShockFront_Pin = 16;    // Belongs to 0x08
const int leftShockFront_Pin = 17;     // Belongs to 0x08

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

// Global message declarations
CAN_message_t msgFront; 
CAN_message_t msgRear;  

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
  
  // Setup Message Headers
  msgFront.id = 0x08; // GPIO FRONT
  msgRear.id = 0x09;  // GPIO REAR

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
  
  // Reset counters for speed calculation
  wheelSpeedRear_Pulses = 0; 
  wheelSpeedFront_Pulses = 0;
  interrupts();

  // MESSAGE 1: GPIO FRONT (0x08)
  msgFront.len = 5;
  
  // Byte 0: diffStateFront (Sending 0 as a placeholder)
  msgFront.buf[0] = 0x00; 

  // Bytes 1 and 2: Right Shock Front
  uint16_t rsf_16bit = (uint16_t)rightShockFront_Total;
  msgFront.buf[1] = (rsf_16bit >> 8) & 0xFF;
  msgFront.buf[2] = rsf_16bit & 0xFF;
  
  // Bytes 3 and 4: Left Shock Front
  uint16_t lsf_16bit = (uint16_t)leftShockFront_Total;
  msgFront.buf[3] = (lsf_16bit >> 8) & 0xFF;
  msgFront.buf[4] = lsf_16bit & 0xFF;

  can1.write(msgFront);

  // GPIO REAR (0x09)
  msgRear.len = 7;

  // Byte 0: diffStateRear (Sending 0 as a placeholder)
  msgRear.buf[0] = 0x00;

  // Bytes 1, 2, 3, 4: Wheel Speed Rear (Float conversion)
  float speedFloat = (float)currentPulsesRear; 
  union {
    float f_val;
    uint8_t b_val[4];
  } floatToBytes;
  floatToBytes.f_val = speedFloat;

  msgRear.buf[1] = floatToBytes.b_val[0];
  msgRear.buf[2] = floatToBytes.b_val[1];
  msgRear.buf[3] = floatToBytes.b_val[2];
  msgRear.buf[4] = floatToBytes.b_val[3];

  // Bytes 5 and 6: Wheel Speed Front
  uint16_t wsf_16bit = (uint16_t)currentPulsesFront;
  msgRear.buf[5] = (wsf_16bit >> 8) & 0xFF;
  msgRear.buf[6] = wsf_16bit & 0xFF;

  can1.write(msgRear);
    
  delay(100);
}
