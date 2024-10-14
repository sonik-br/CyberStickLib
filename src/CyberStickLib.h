/*******************************************************************************
 * CyberStick controller input library.
 * https://github.com/sonik-br/CyberStickLib
 * 
 * The library depends on greiman's DigitalIO library.
 * https://github.com/greiman/DigitalIO
 * 
 * I recommend the usage of SukkoPera's fork of DigitalIO as it supports a few more platforms.
 * https://github.com/SukkoPera/DigitalIO
 * 
 * THIS LIB IS UNTESTED ON REAL HARDWARE
*/

#include <DigitalIO.h>

#ifndef CYBERSTICKLIB_H_
#define CYBERSTICKLIB_H_

enum CyberStickDigital_Enum {
  CYBERSTICK_D      = 0x01,
  CYBERSTICK_C      = 0x02,
  CYBERSTICK_B      = 0x04,
  CYBERSTICK_A      = 0x08,
  CYBERSTICK_SELECT = 0x10,
  CYBERSTICK_START  = 0x20,
  CYBERSTICK_E2     = 0x40,
  CYBERSTICK_E1     = 0x80
};

enum CyberStickAnalog_Enum {
  CYBERSTICK_CH0 = 0,  // stick vertical
  CYBERSTICK_CH1,      // stick horizontal
  CYBERSTICK_CH2       // slider
};

struct CyberStickControllerState {
  bool connected = false;
  uint8_t digital = 0xff;
  uint8_t ch0 = 0x80;
  uint8_t ch1 = 0x80;
  uint8_t ch2 = 0x80;

  bool operator!=(const CyberStickControllerState& b) const {
    return connected != b.connected ||
         digital != b.digital ||
         ch0 != b.ch0 ||
         ch1 != b.ch1 ||
         ch2 != b.ch2;
  }
};

template <uint8_t PIN_D0, uint8_t PIN_D1, uint8_t PIN_D2, uint8_t PIN_D3, uint8_t PIN_REQ, uint8_t PIN_ACK, uint8_t PIN_LH>
class CyberStickInput {
  private:
    DigitalPin<PIN_D0>  pin_D0;
    DigitalPin<PIN_D1>  pin_D1;
    DigitalPin<PIN_D2>  pin_D2;
    DigitalPin<PIN_D3>  pin_D3;
    DigitalPin<PIN_REQ> pin_REQ;
    DigitalPin<PIN_ACK> pin_ACK;
    DigitalPin<PIN_LH>  pin_LH;

    CyberStickControllerState currentState;
    CyberStickControllerState lastState;

    uint8_t errorCounter = 0;

    inline uint8_t __attribute__((always_inline)) 
    readNibble() { return (pin_D3 << 3) + (pin_D2 << 2) + (pin_D1 << 1) + pin_D0; }

    inline void __attribute__((always_inline)) 
    writeRequest(uint8_t value) { pin_REQ.write(value); }

    inline uint8_t __attribute__((always_inline)) 
    waitLH(uint8_t state) { //returns 1 when reach timeout
      uint16_t t_out = 2000;
      const uint8_t compare = !state;
      while (pin_LH == compare) {
        delayMicroseconds(1);
        if (t_out-- == 1)
          return 1;
      }
      return 0;
    }

    inline uint8_t __attribute__((always_inline)) 
    waitACK(uint8_t state) { //returns 1 when reach timeout
      uint16_t t_out = 2000;
      const uint8_t compare = !state;
      while (pin_ACK == compare) {
        delayMicroseconds(1);
        if (t_out-- == 1)
          return 1;
      }
      return 0;
    }

  public:
    bool connectionJustChanged() const { return currentState.connected != lastState.connected; }
    bool stateChanged() const { return currentState != lastState; }
    
    uint16_t digitalRaw() const { return currentState.digital; }  

    bool isConnected() const { return currentState.connected; }
    bool digitalPressed(const CyberStickDigital_Enum s) const { return (currentState.digital & s) == 0; }
    bool digitalChanged (const CyberStickDigital_Enum s) const { return ((lastState.digital ^ currentState.digital) & s) > 0; }
    bool digitalJustPressed (const CyberStickDigital_Enum s) const { return digitalChanged(s) & digitalPressed(s); }
    bool digitalJustReleased (const CyberStickDigital_Enum s) const { return digitalChanged(s) & !digitalPressed(s); }

    bool analogChanged (const CyberStickAnalog_Enum s) const {
        switch (s) {
          case CYBERSTICK_CH0: return currentState.ch0 != lastState.ch0;
          case CYBERSTICK_CH1: return currentState.ch1 != lastState.ch1;
          case CYBERSTICK_CH2: return currentState.ch2 != lastState.ch2;
          default: return false;
      }
    }

    uint8_t analog(const CyberStickAnalog_Enum s) const {
      switch (s) {
        case CYBERSTICK_CH0: return currentState.ch0;
        case CYBERSTICK_CH1: return currentState.ch1;
        case CYBERSTICK_CH2: return currentState.ch2;
        default: return 0;
      }
    }
    
    void begin(){
      //init output pins
      pin_REQ.config(OUTPUT, HIGH);
    
      //init input pins with pull-up
      pin_D0.config(INPUT, HIGH);
      pin_D1.config(INPUT, HIGH);
      pin_D2.config(INPUT, HIGH);
      pin_D3.config(INPUT, HIGH);
      pin_LH.config(INPUT, HIGH);
      pin_ACK.config(INPUT, HIGH);

      errorCounter = 0;
    }

    void update(){
      uint8_t data[12];
      data[4] = 0xf; // Start by setting invalid data. Should clear when reading
      data[8] = 0xf; // Those are ch3 data on cyberstick? not sure
      bool deviceClockState = 0;

      // Sometimes it miss this ACK on the last nibble (i == 11)
      // It's safe to ignore?
      bool error = false;

      // copy current values
      lastState.connected = currentState.connected;
      lastState.digital = currentState.digital;
      lastState.ch0 = currentState.ch0;
      lastState.ch1 = currentState.ch1;
      lastState.ch2 = currentState.ch2;
    
      if (pin_ACK == 0x1 && pin_LH == 0x0) { // need testing
        for (uint8_t i=0; i<12; ++i) {

          // request next byte
          if (i < 11 && (i & 1) == 0){
            writeRequest(LOW);
            if (waitACK(LOW) && i < 11) {
              error = true;
              break;
            }
            writeRequest(HIGH);
          }
          
          if (waitACK(LOW) && i < 11) {
            error = true;
            break;
          }
          
          if (waitLH(deviceClockState) && i < 11) {
            error = true;
            break;
          }
          
          data[i] = readNibble();
          
          if (waitACK(HIGH) && i < 11) {
            error = true;
            break;
          }
          deviceClockState = !deviceClockState;
        } // end loop
      }

      //got any error?
      if (error || data[4] || data[8]) {
        if (++errorCounter > 4) {
          errorCounter = 0;
          currentState.connected = false;
        }
      } else {
        errorCounter = 0;
        currentState.connected = true;
        currentState.digital = (data[0] << 4) | data[1];
        currentState.ch0     = (data[3] << 4) | data[7];
        currentState.ch1     = (data[2] << 4) | data[6];
        currentState.ch2     = (data[5] << 4) | data[9];
      }
      
      // reset request
      writeRequest(HIGH);
      delayMicroseconds(4);
    }
};

#endif
