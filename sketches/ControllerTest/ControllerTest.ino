/*******************************************************************************
 * CyberStick controller input library.
 * https://github.com/sonik-br/SaturnLib
 * 
 * The library depends on greiman's DigitalIO library.
 * https://github.com/greiman/DigitalIO
 * 
 * I recommend the usage of SukkoPera's fork of DigitalIO as it supports a few more platforms.
 * https://github.com/SukkoPera/DigitalIO
 * 
 * 
 * This sketch is ready to use on a Leonardo but may work on any
 * arduino with the correct number of pins and proper setup.
 * 
 * THIS LIB IS UNTESTED ON REAL HARDWARE
 * 
*/

#include <CyberStickLib.h>

/*
DE9 pinout reference

CyberStick CZ-8NJ2 / XE-1 AJ (J-PC)
1 D0
2 D1
3 D2
4 D3
6 LH
7 ACK
8 REQ

XE-1 AP (MD)
1 D0  (UP)
2 D1  (DOWN)
3 D2  (LEFT)
4 D3  (RIGHT)
6 LH  (TL)
7 REQ (TH)
9 ACK (TR)
*/

// GPIO
#define INPUT_REQ A2
#define INPUT_ACK 7
#define INPUT_LH  A1
#define INPUT_D0  A4
#define INPUT_D1  A5
#define INPUT_D2  A3
#define INPUT_D3  A0

CyberStickInput<INPUT_D0,INPUT_D1,INPUT_D2,INPUT_D3,INPUT_REQ,INPUT_ACK,INPUT_LH> cyberStick;

#define DIGITALSTATE(D) \
if(cyberStick.digitalJustPressed(D)) { \
  Serial.println(F("Digital pressed: " #D)); \
} else if(cyberStick.digitalJustReleased(D)) {\
  Serial.println(F("Digital released: " #D)); \
}

#define ANALOGSTATE(A) \
if(cyberStick.analogChanged(A)) {\
  Serial.print(F("Analog " #A ": ")); \
  Serial.println(cyberStick.analog(A)); \
}

void printButtons() {
  DIGITALSTATE(CYBERSTICK_D)
  DIGITALSTATE(CYBERSTICK_C)
  DIGITALSTATE(CYBERSTICK_B)
  DIGITALSTATE(CYBERSTICK_A)
  DIGITALSTATE(CYBERSTICK_SELECT)
  DIGITALSTATE(CYBERSTICK_START)
  DIGITALSTATE(CYBERSTICK_E2)
  DIGITALSTATE(CYBERSTICK_E1)
}

void printAnalog() {
  ANALOGSTATE(CYBERSTICK_CH0)
  ANALOGSTATE(CYBERSTICK_CH1)
  ANALOGSTATE(CYBERSTICK_CH2)
}

void setup() {
  Serial.begin(115200);
  // Init the library.
  cyberStick.begin();
  delay(100);
}

void loop() {
  // Call update to read the controller
  cyberStick.update();

  if (cyberStick.connectionJustChanged()) {
    if (cyberStick.isConnected())
      Serial.println(F("Device connected."));
    else
      Serial.println(F("Device disconnected."));
    delay(16);
  }

  if (cyberStick.isConnected() && cyberStick.stateChanged()) {
    printButtons();
    printAnalog();
  }
  
  //delay(16);
  delay(4);
}
