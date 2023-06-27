#ifndef SPEED_CONTROLLER_H 
#define SPEED_CONTROLLER_H

#include <Arduino.h>

enum SpeedControlMode {
  PADDLE_SPEED = 0, 
  BUFFER_SPEED = 1,
  COMMAND_SPEED = 2
};

class SpeedController {
  protected:
    int value = 20     ; // current speed in WPM
    byte minimumWk =  5 ; // minimum value for WK2 protocol
    byte minValue  =  5 ; // minimum value for potentiometer
    byte maxValue  = 40 ; // maximum value
  public:
    virtual void init() ;  // method to be overridden by implementation according to hardware
    virtual void update(); // method to be overridden by implementation according to hardware
    virtual void setValue(int); // 
    void setMinMax( byte min, byte max) ;
    byte getValue() ;
    byte getSpeedWk2() ;
};

#endif