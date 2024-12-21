#include "speed_controller.h"

/* This class implements unspecific methods 
   to set, get minimum and maximum values,
   set WPM for every mode of operation
   (paddles, )
*/

byte SpeedController::getValue() { return value; }

// virtual method - to be overridden by derived class
void SpeedController::init() {}

void SpeedController::setButtonPin( byte pin ) {
  button = pin ;
}
// virtual method - to be overridden by derived class
void SpeedController::update() {}
// virtual method - to be overridden, only to be used with RotaryEncoder
void SpeedController::setValue(int v) {}
// 
void SpeedController::setMinMax(byte min, byte max) {
  if( min >= 5 ) { minValue = min ; minimumWk = min ; }
  if( max > min && max >= 15 )  maxValue = max ; 
}

byte SpeedController::getSpeedWk2() {
  return (((value - minValue) & 0x3F) | 0x80 );
}

byte SpeedController::getEvent()
{
  byte e = buttonEvent ;
  buttonEvent = 0 ;
  return e ;
}

/**
 * Check button state, use debounce counter, generate event when necessary
 */
void SpeedController::checkButton() {
  if( button > 0 ) {
    if( debounceCounter > 0 ) debounceCounter-- ; // while debounce counter is running, do nothing
    else {
      byte b = digitalRead( button ); // read current state of the button 
      switch( buttonState ) {
        case 0: // button was released
          if( b == 1 ) { // now it is pressed: start debounce period
            debounceCounter = DEFAULT_DEBOUNCE_PERIOD ;
          }
          break ; // if the button was not pressed and remains not pressed, nothing to do
        case 1: // button was pressed
          if( b == 0 ) { // button was released after press
            buttonEvent = 1 ;
          }
      }
      buttonState = b ;
    }
  }
}
