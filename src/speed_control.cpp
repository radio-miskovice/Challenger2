#include "config_speedcontrol.h"

#if defined (CONFIG_SPEED_TYPE_ROTARY)

// use rotary encoder as speed control
#include "rotary_encoder.h"
SpeedController *speedControl = &encoder;

#elif defined (CONFIG_SPEED_TYPE_POTENTIOMETER)

// use potentiometer as speed control
#include "potentiometer.h"
SpeedController *speedControl = &potentiometer;

#endif // USE_POTENTIOMETER
