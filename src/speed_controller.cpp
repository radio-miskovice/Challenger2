#include "speed_controller.h"

/* This class implements unspecific methods 
   to set, get minimum and maximum values,
   set WPM for every mode of operation
   (paddles, )
*/

byte SpeedController::getValue() { return value; }

// virtual method - to be overridden by derived class
void SpeedController::init() {}
// virtual method - to be overridden by derived class
void SpeedController::update() {}
// virtual method - to be overridden, only to be used with RotaryEncoder
void SpeedController::setValue(int v) { }
// 
void SpeedController::setMinMax(byte min, byte max) {
  if( min >= 5 ) { minValue = min ; minimumWk = min ; }
  if( max > min && max >= 15 )  maxValue = max ; 
}
