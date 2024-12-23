#include "config_speedcontrol.h"

#if defined(POTENTIOMETER)

#include <Arduino.h>
#include "pins.h"
#include "core.h"
#include "core_variables.h"

#include "potentiometer.h"

#if defined(PIN_POTENTIOMETER) && PIN_POTENTIOMETER > 0
#define HAS_POTENTIOMETER
#endif

Potentiometer potentiometer = Potentiometer();

void Potentiometer::init() {
  #ifdef HAS_POTENTIOMETER
  pinMode(PIN_POTENTIOMETER, INPUT);
  value = analogRead(PIN_POTENTIOMETER);
  value = map(value, 0, POT_FULL_SCALE, minValue, maxValue );
  #endif
  lastMeasurementMs = millis() - POT_INTERVAL_MS - 1;
}

void Potentiometer::update() {
  if( millis() - lastMeasurementMs > POT_INTERVAL_MS ) {
    word measuredValue = 0 ;
    byte oldValue = value ;
    lastMeasurementMs = millis();
    #ifdef HAS_POTENTIOMETER
    value = analogRead(PIN_POTENTIOMETER);
    value = map( measuredValue, 0, POT_FULL_SCALE, minValue, maxValue );
    #endif
    hasChanged = ( oldValue != value ); 
  }
}

#endif