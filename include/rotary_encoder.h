/* ROTARY ENCODER VARIABLES */
#if !defined(ROTARY_ENCODER_H)
#define ROTARY_ENCODER_H 

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