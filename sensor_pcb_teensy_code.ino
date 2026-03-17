#include <FlexCAN_T4.h>

// Defining pins
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
CAN_message_t msg; // create CAN message

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
  
  // Attach interrupts to trigger on the falling edge of the pulse, triggers the wheel speed low reading before the 100 ms if the car is going fast enough
  attachInterrupt(digitalPinToInterrupt(wheelSpeedSensor1_Pin), countPulse1, FALLING);
  attachInterrupt(digitalPinToInterrupt(wheelSpeedSensor2_Pin), countPulse2, FALLING);

  // Set analog pins as inputs
  pinMode(shockTravelSensor1_Pin, INPUT);
  pinMode(shockTravelSensor2_Pin, INPUT);

  // Setup CAN Bus, not really sure if this is the correct way to do can
  can1.begin();
  can1.setBaudRate(500000); // 500kbps
  // static parts of CAN
  msg.id = 0x100 // CAN ID for board, this might need to change
  msg.len = 8 // 8 bytes
  }
void loop() {
  // Read Analog Shock Sensors (Returns a value from 0 to 1023 because of 12-bit onboard ADC in Teensy)
  shockTravel1_Val = analogRead(shockTravelSensor1_Pin);
  shockTravel2_Val = analogRead(shockTravelSensor2_Pin);

  // Get nterrupt counters
  noInterrupts();
  unsigned long currentPulses1 = wheelSpeed1_Pulses;
  unsigned long currentPulses2 = wheelSpeed2_Pulses;
  interrupts();

  // CAN message
  // bytes 0 and 1 are shock travel 1
  msg.buf[0] = (shockTravel1_Val >> 8) & 0xFF;
  msg.buf[1] = shockTravel1_Val & 0xFF;
  // bytes 2 and 3 are shock travel 2
  msg.buf[2] = (shockTravel2_Val >> 8) & 0xFF;
  msg.buf[3] = shockTravel2_Val & 0xFF;
  // bytes 4 and 5 are ws 1
  uint16_t ws1_16bit = (uint16_t)currentPules1;
  msg.buf[4] = (ws1_16bit >> 8) & 0xFF;
  msg.buf[5] = ws1_16bit & 0xFF;
  // bytes 6 and 7 are ws 2
  uint16_t ws2_16bit = (uint16_t)currentPules1;
  msg.buf[6] = (ws2_16bit >> 8) & 0xFF;
  msg.buf[7] = ws2_16bit & 0xFF;
  // send to cAN
  can1.write(msg)
    
  delay(100); // Wait 100 milliseconds before reading again, only used if interrupt os not reached
}
}
