#ifndef _SPEED_CONTROLLER_H_
#define _SPEED_CONTROLLER_H_

#include <Arduino.h>
/*
  This file defines base class for speed controlling device. Such device 
  could typically be 1. potentiometer + push button, 2. rotary encoder with push button
  or 3. a set of two or three buttons.
  The base class keeps speed variables (actual, minimum, maximum, minimum for WK2 protocol)
  as well as button-related data and methods.
  This base class implements only methods common to any speed controlling device,
  while the device-specific methids must be implemented in derived classes.
*/

enum SpeedControlMode {
  PADDLE_SPEED = 0, 
  BUFFER_SPEED = 1,
  COMMAND_SPEED = 2
};

const byte DEFAULT_DEBOUNCE_PERIOD = 12 ;

class SpeedController {
  protected:
    int value = 20     ; // current speed in WPM
    byte minimumWk =  5 ; // minimum value for WK2 protocol
    byte minValue  =  5 ; // minimum value for potentiometer
    byte maxValue  = 40 ; // maximum value
    byte buttonState = 0 ; // 1 = pressed, 0 = released
    byte prevButtonState = 0 ; // same as above, but before last change
    byte debounceCounter = 0 ; // countdown of debounce period
    byte buttonCounter = 0 ;   // counter of consecutive equal button states
    byte button = 0 ;          // button pin (Arduino number)
    byte buttonEvent = 0 ;     // 1 = click, 2 = double click
    void setButtonPin( byte pin );
  public:
    virtual void init() ;  // method to be overridden by implementation according to hardware
    virtual void update(); // method to be overridden by implementation according to hardware
    virtual void setValue(int); // set current speed value
    void checkButton() ; // check if the button changed state; must be called only when milliseconds change
    byte getEvent() ;    // returns current event and clears event memory
    void setMinMax( byte min, byte max) ;
    byte getValue() ;
    byte getSpeedWk2() ;
};

#endif