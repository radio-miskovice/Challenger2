/* ROTARY ENCODER VARIABLES */
#if !defined(_ROTARY_ENCODER_H_)
#define _ROTARY_ENCODER_H_

#include "config_speedcontrol.h"
#include "speed_controller.h"
#include <Arduino.h>

class RotaryEncoder : public SpeedController {
  private: 
    static void enableInterrupt();
    static void disableInterrupt();
    int valueIncrement = 0;
    int cropValue(int);
  public: 
    void init() override;
    void update() override;
    void setValue(int v) override;
} ;

extern RotaryEncoder encoder ;

#endif 