#include "paddle.h"

/**
 * Initialize paddle interface ports. Mode setting is not included, setMode(newMode) must be 
 * called separately.
 */
void PaddleInterface::init() {

  pinMode( pinPaddleRight, INPUT );
  pinMode( pinPaddleLeft, INPUT );
}

/**
 * Toggle swap paddle assignment
 */
void PaddleInterface::swap() {
  swapPaddle = !swapPaddle ;
}

/**
 * Checks paddle interface ports.
 * If nothing changed, do nothing.
 * Otherwise check if paddles were touched - this will interrupt buffer text send if needed.
 * If in Ultimatic mode, evaluate whether next element has to be changed and change it (special logic). 
 * If in iambic mode, set next element by paddle state, or put opposite to current if squeeze detected.
 * If in iambic A and paddles have no contact, delete element buffer (set next element to NO_ELEMENT)
 * @return updated status
 */
byte PaddleInterface::check() {
    byte portBits = digitalRead(pinPaddleLeft) * DIT + digitalRead(pinPaddleRight) * DAH ;
    portBits = portBits ^ 3 ;
    if( swapPaddle ) {
      portBits = DAH * (portBits & DIT ? 1 : 0 ) + DIT * (portBits & DAH ? 1 : 0 );
    }
    return portBits ;
}

PaddleInterface paddle; // PaddleInterface singleton instance