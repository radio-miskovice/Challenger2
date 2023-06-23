/**
 * Rotary encoder variables and handling 
 * [CC BY-NC-4.0] Creative commons Licence 4.0
 * https://creativecommons.org/licenses/by-nc/4.0/
 * Jindrich Vavruska, jindrich@vavruska.cz
 **/
#include <avr/io.h>
#include <Arduino.h>

#include "config_speedcontrol.h"
#include "rotary_encoder.h"

#if defined (CONFIG_SPEED_TYPE_ROTARY)

/* Rotary encoder vector and mask calculation */
#if CONFIG_SPEED_ROTARY_CLOCK > 1 && CONFIG_SPEED_ROTARY_CLOCK < 8
#define ROT_INT_VECTOR PCINT2_vect
#define ROT_INT_MASK_REG PCMSK2
#define ROT_INT_MASK (1 << (PIN_ROTARY_CLOCK))
#define ROT_PCIE_MASK (1 << PCIE2)
#define ROT_PINS PIND
#define ROT_PC_INTERRUPT

#elif CONFIG_SPEED_ROTARY_CLOCK > 7 && CONFIG_SPEED_ROTARY_CLOCK < 14
#define ROT_INT_VECTOR PCINT0_vect
#define ROT_INT_MASK_REG PCMSK0
#define ROT_INT_MASK (1 << (CONFIG_SPEED_ROTARY_CLOCK - 8))
#define ROT_PCIE_MASK (1 << PCIE0)
#define ROT_PINS PINB
#define ROT_PC_INTERRUPT

#elif CONFIG_SPEED_ROTARY_CLOCK > 13 && CONFIG_SPEED_ROTARY_CLOCK < 20
#define ROT_INT_VECTOR PCINT1_vect
#define ROT_INT_MASK_REG PCMSK1
#define ROT_INT_MASK (1 << (PIN_ROTARY_CLOCK - 14))
#define ROT_PCIE_MASK (1 << PCIE1)
#define ROT_PINS PINC
#define ROT_PC_INTERRUPT
#else
#error "Cannot determine interrupt configuration for rotary encoder"
#endif

#if CONFIG_SPEED_ROTARY_DATA < 8
#define ROT_VALUE_SHIFT CONFIG_SPEED_ROTARY_DATA
#elif PIN_ROTARY_VALUE < 14
#define ROT_VALUE_SHIFT CONFIG_SPEED_ROTARY_DATA - 8
#else
#define ROT_VALUE_SHIFT CONFIG_SPEED_ROTARY_DATA - 14
#endif

RotaryEncoder encoder ;

volatile bool isEventPending = false ; // indicates need to update rotary encoder value
volatile int  interruptIncrement = 0 ; // increment accumulated from ISR

/**
 * Initialize ports and enable interrupt
 */
void RotaryEncoder::init()
{
  pinMode(CONFIG_SPEED_ROTARY_CLOCK, INPUT_PULLUP);
  pinMode(CONFIG_SPEED_ROTARY_DATA, INPUT_PULLUP);
  enableInterrupt();
}

/**
 * Ensure that value returned is within limits
 * @param v value intended to be set
 * @return if v is within limits, returns v, otherwise the low or high limit
*/
int RotaryEncoder::cropValue( int v ) {
  if (v < minValue) return minValue ;
  if (v > maxValue) return maxValue ;
  return v ;
}

/**
 * @param v new value to be set.
*/
void RotaryEncoder::setValue( int v ) {
  value = cropValue(v);
}

/**
 * Update speed control value. In case of rotary encoder check if there are
 * any data collected by interrupt routine and update the value accordingly.
 */
void RotaryEncoder::update()
{
  // if(!isEventPending) return ; // do nothing if nothing happened
  if( interruptIncrement == 0 ) return ;
  byte _pcicr = PCICR & ROT_PCIE_MASK ; // remember original interrupt state
  PCICR &= ~ROT_PCIE_MASK; // disable interrupt while reading 
  valueIncrement = interruptIncrement ;
  interruptIncrement = 0;
  isEventPending = false ;
  if( value + valueIncrement > 0 ) {
    value = cropValue(value + valueIncrement) ;
  }
  PCICR = PCICR | _pcicr; // return original value - enable interrupt if previously enabled
}

/**
 * Enables interrupt to act
 */
void RotaryEncoder::enableInterrupt() {
  ROT_INT_MASK_REG = ROT_INT_MASK;
  PCICR |= ROT_PCIE_MASK;
}

/** Disables interrupt */
void RotaryEncoder::disableInterrupt()
{
  PCICR &= ~ROT_PCIE_MASK;
}

ISR(ROT_INT_VECTOR)
{
  // if( !isEventPending ) {
    byte state = ROT_PINS ;
    if( state & ROT_INT_MASK ) {
      state = (state >> (ROT_VALUE_SHIFT)) & 1 ;
      interruptIncrement += ( 1 - 2*state );
      isEventPending = true ;
    }
  //}
}
#endif