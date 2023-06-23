#ifndef _CONFIG_SPEEDCONTROL_H_
#define _CONFIG_SPEEDCONTROL_H_

// undefine or set to 0 if there is no speed control
#define CONFIG_SPEEDCONTROL_USE   1

// choose ROTARY for rotary encoder or POTENTIOMETER for potentiometer
// uncomment the one in use and comment the other one
#define CONFIG_SPEED_TYPE_ROTARY
// #define CONFIG_SPEED_TYPE_POTENTIOMETER

/** ----- Rotary encoder configuration parameters ----- **/

// choose pin to be bound to interrupt
#define CONFIG_SPEED_ROTARY_CLOCK 10

// choose pin to provide UP/DOWN data
#define CONFIG_SPEED_ROTARY_DATA 11

// choose pin to act as switch (digital), or zero if used as analog or not used at all
#define CONFIG_SPEED_ROTARY_BUTTON_DIGITAL 7

// choose pin to act as analog button, or 0 if not used
#define CONFIG_SPEED_ROTARY_BUTTON_ANALOG 0
// analog button threshold
#define CONFIG_SPEED_ROTARY_BUTTON_THRESHOLD 0

/** ----- Potentiometer configuration parameters ----- **/
// choose analog pin; if CONFIG_SPEED_TYPE == "ROTARY", this symbol is ignored
#define CONFIG_SPEED_POT_INPUT 0

#endif 
