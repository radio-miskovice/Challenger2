#include "paddle.h"

/**
 * Initialize paddle interface ports. Mode setting is not included, setMode(newMode) must be 
 * called separately.
 */
void PaddleInterface::init() {
  pinMode( pinPaddleRight, INPUT_PULLUP );
  pinMode( pinPaddleLeft, INPUT_PULLUP );
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
PaddleStatus PaddleInterface::check() {
  // TODO: timing check should be used to debounce?
  byte paddlePortBits = digitalRead(pinPaddleLeft) * DIT + digitalRead(pinPaddleRight) * DAH ;
  paddlePortBits = paddlePortBits ^ 0x03 ; // reverse active bit values to facilitate analysis
  if( paddlePortBits == lastPaddlePortBits ) return status ; // nothing changed, no action required
  status.touch = (paddlePortBits > 0) ? ON : status.touch ; // record touch for case buffer text is played; touch interrupts

  // ULTIMATIC handling
  // do only when ULTIMATIC bits are different from read result
  if( status.mode == ULTIMATIC && ultimusBits == paddlePortBits ) {
    // 00: no previous contact
    if( ultimusBits == 0) {
      if( paddlePortBits == DIT + DAH ) ultimusBits = DAH ; // rare squeeze condition, choose DAH
      else ultimusBits = paddlePortBits ; // the active contact wins
    }
    else { // previous contact exists 
      ultimusBits = (ultimusBits ^ paddlePortBits) & paddlePortBits ; 
    }
    // set element resulting from the new paddle state
    // we will take advantage of enum values corresponding with the bits involved
    status.next = (ElementType) ultimusBits ;
  }
  // IAMBIC_A and IAMBIC_B
  else {
    // clear next element memory on no contact and IAMBIC A
    if( paddlePortBits == 0 && status.mode == IAMBIC_A ) status.next = NO_ELEMENT ;
    if( paddlePortBits == DIT + DAH ) {
      if( status.current == NO_ELEMENT ) status.next = DAH ; // again the rare condition never to be met in reality
      else status.next = (status.current == DAH) ? DIT : DAH ; // otherwise change the element
    }
    else
      status.next = (ElementType) paddlePortBits;
  }
  lastPaddlePortBits = paddlePortBits ; // remember new value
  return status ; // return new status; not consumed except when playing text buffer (touch has to interrupt it)
}

/**
 * @return status with the next element to be played (status.current ! not status.next !!! it returns after the change)
 */
PaddleStatus PaddleInterface::getNext() {
  status.current = status.next ; // keep "next" continuously, unless it has to change 
  // if still in squeeze condition and any IAMBIC mode, change status.next accordingly!
  if( (lastPaddlePortBits ==  DIT+DAH) && status.mode != ULTIMATIC )
    status.next = status.current == DAH ? DIT : DAH ;
    status.current = NO_ELEMENT ;
  return status ;
}

/**
 * Set new mode if it is not the same as current mode.
 * Setting new mode resets all paddle state variables.
 */
PaddleStatus PaddleInterface::setMode(KeyerModeEnum newMode) {
  if( status.mode == newMode ) return status ;
  status.mode = newMode ;
  status.current = NO_ELEMENT ;
  status.next = NO_ELEMENT ;
  lastPaddlePortBits = 0 ;
  ultimusBits = 0;
  return status ;
}

PaddleInterface paddle; // PaddleInterface singleton instance