
#ifndef _POTENTIOMETER_H_
#define _POTENTIOMETER_H_

#include "config_speedcontrol.h"
#include <Arduino.h>
#include "speed_controller.h"

// minimum time to elapse between measurements
#define POT_INTERVAL_MS 150

#ifdef __LGT8FX8P__
#define POT_FULL_SCALE ((1<<12) - 1)
#else
#define POT_FULL_SCALE  (1023)
#endif

class Potentiometer : public SpeedController
{
  private:
  unsigned long lastMeasurementMs = 0 ;
  public:
  void init() override ;
  void update() override;
};

#if CONFI
extern Potentiometer potentiometer ;
#endif

#endif
