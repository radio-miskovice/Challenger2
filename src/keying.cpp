#include <Arduino.h>
#include "keying.h"

// Keying interface singleton
KeyingInterface keyer = KeyingInterface() ;

/**
 * initializes Arduino ports
 */
void KeyingInterface::init()
{
  if (_keyline1 > 0)  pinMode(_keyline1, OUTPUT);
  if (_pttline1 > 0)  pinMode(_pttline1, OUTPUT);
  if (_sidetone > 0)  pinMode(_sidetone, OUTPUT);
  if (_cpo_key  > 0)  pinMode(_cpo_key, OUTPUT);
  onTimer = 0UL ;
  offTimer = 0UL ;
  status.busy = IDLE ;
}

/**
 * set Key line.
 * @param level - high or low
 */
void KeyingInterface::setKey(OnOffEnum onOff)
{
  if( flags.key == ENABLED ) {
    digitalWrite(_keyline1, onOff);
    status.key = onOff;
  } 
  else {
    digitalWrite(_keyline1, LOW);
    status.key = OFF ;
  }
}

/**
 * @param level - high or low
*/
void KeyingInterface::setPtt(OnOffEnum onOff)
{
  if( flags.ptt == ENABLED ) {
    digitalWrite(_pttline1, onOff);
    status.ptt = onOff ;
  }
  else {
    digitalWrite(_pttline1, LOW);
    status.ptt = OFF ;
  }
}

/**
 * @param hz when hz = 0 stops sidetone, otherwise starts sidetone with frequency hz Hz
*/
void KeyingInterface::setTone(word hz)
{
  hz = trimToneFreq(hz) ;
  if( hz > 0)
    tone(_sidetone, hz); 
  else noTone(_sidetone);
}

/**
 * @param hz required sidetone frequency
 * @returns sidetone frequency trimmed to remain between limits
 */
word KeyingInterface::trimToneFreq(word hz)
{
  if( hz == 0 ) return 0 ;
  return ( hz < minToneFreq ? minToneFreq : ( hz > maxToneFreq ? maxToneFreq : hz ));
}

/**
 *  @param  hz sidetone frequency to set for high-level sending
 */
void KeyingInterface::setToneFreq( word hz ) {
  hz = trimToneFreq(hz);
  if( hz > 0 ) toneFreq = hz ;
}

void KeyingInterface::setTimingParameters( byte wpm, word _dahRatio, word _weighting ) {
  unit = 1200 / wpm ;
  ditDahFactor = (_dahRatio == 0) ? ditDahFactor : _dahRatio;
  weighting = (_weighting == 0) ? weighting : _weighting;
}

void KeyingInterface::enableKey(EnableEnum enable)
{
  flags.key  = enable  ; 
  if( flags.key == DISABLED ) setKey(OFF);
}

void KeyingInterface::enablePtt(EnableEnum enable)
{
  flags.ptt  = enable  ; 
  if( flags.ptt == DISABLED ) setPtt(OFF);
}

void KeyingInterface::enableTone(EnableEnum enable)
{
  flags.tone = enable ; 
  if( flags.tone == DISABLED ) setTone(OFF);
}

/**
 * This method checks time, services output control timers and updates line levels and sidetone as necessary.
 * @return current service status: IDLE (no timing in progress), BUSY (timing in progress), DIT (sending DIT), DAH (sending dah), SPACE (sending char space or wordspace)
*/
KeyingStatus KeyingInterface::service( byte ps ) {
  if( status.busy == IDLE ) {
    sendPaddleElement( ps );
    return status ;
  } 
  unsigned long interval = currentTime - lastMillis ;
  if( interval == 0UL ) return status ; // timing in progress, but elapsed zero time, hence no change

  lastMillis = currentTime ;
  // key on timing
  if( onTimer > 0UL ) {
    if( onTimer < interval ) onTimer = 0 ; 
    else onTimer = onTimer - interval ;
    if( onTimer == 0 ) {
      setKey( OFF );
      setTone( 0 );
      lastBPaddle = PADDLE_FREE ;
    }
    return status ;
  }

  // key off timing 
  if( offTimer > 0 ) {
    lastBPaddle = lastBPaddle | ps ;
    if( offTimer < interval ) offTimer = 0 ;
    else offTimer = offTimer - interval ;
    if( offTimer == 0 ) { 
       status.last = status.current ;
       status.current = NO_ELEMENT ;
       status.busy = IDLE ;
      }
    }
  return status ;
}

/**
 * @param input paddle input: bit 0 = DIT (1), bit 1 = DAH (2), value 3 = squeeze
 */
KeyingStatus KeyingInterface::sendPaddleElement(byte input)
{
  ElementType nextElement;
  if (mode == ULTIMATIC) // input conditioning for Ultimatic mode
  {
      input = (lastUltimaticPaddle ^ input) & input;
      lastUltimaticPaddle = input;
  }
  else if (mode == IAMBIC_B && input == PADDLE_FREE) // use memory in case of Iambic B mode
  {
      input = lastBPaddle;
      lastBPaddle = PADDLE_FREE;
  }
  switch (input) 
  {
  case 3: // squeeze; after previous transformation, ultimatic will never have 3
      nextElement = (status.last == DIT) ? DAH : DIT;
      break;
  case 0:
      nextElement = NO_ELEMENT;
      break; // NO_ELEMENT is already in NextElement, which would be the "else" branch
  default:   // the rest is either DIT or DAH
      nextElement = (ElementType)input;
  }
  return (sendElement(nextElement));
}

/***
 * Set element timers and status according to element.
 * @param element new current element to set up
 */
KeyingStatus KeyingInterface::sendElement( ElementType element, bool addCharSpace ) {
  word elementFactor = 100 ;   // 100 for DIT, ditDahFactor for DAH 
  word extraSpaceFactor = 2 ; // 2 for CHARSPACE, 4 for WORDSPACE
  status.current = element ; // set new current element
  status.busy = BUSY ;      // set new status 
  lastBPaddle = PADDLE_FREE ;
  switch( element ) {
    case NO_ELEMENT:
      status.busy = IDLE ;
      onTimer = 0UL ;
      offTimer = 0UL ;
      break ;
    case DAH:
      elementFactor = ditDahFactor ;
    case DIT:
      onTimer = ( unit * weighting ) / 50UL ; // DIT duration with weighting
      offTimer = 2 * unit - onTimer ;  // element space duration with weighting
      if( addCharSpace ) {
        offTimer += 2 * unit ;
      }
      // now comes the magic for DAH: multiply by ditDahFactor, but keep current for DIT
      onTimer = (onTimer * elementFactor) / 100UL ;
      setKey( ON );
      setTone( toneFreq );
      break;

    // word space: add 4T pause after 3T character space
    case WORDSPACE:
      extraSpaceFactor = 4 ;
    // half space: add 3T
    case HALFSPACE:
    // charspace: add 2T pause after the last element
    case CHARSPACE:
      onTimer = 0 ;
      offTimer = unit * extraSpaceFactor + (element == HALFSPACE ? 1 : 0 );
      break; 
  }
  lastMillis = millis();   // initialize reference currentTime for element timers
  return status ;
}

/**
 * Start sending timed element immediately if the keyer is idle,
 * or put new element in waiting queue otherwise.
*/
/*
KeyingStatus KeyingInterface::sendElement( ElementType element, bool isEndOfChar ) {
  // if keyer is IDLE, start new element right off
  if( status.busy == IDLE ) {
    sendElement( element );
  }
  return status ;
}
*/

/**
 * Set new mode 
 */
void KeyingInterface::setMode(KeyerModeEnum newMode)
{
  mode = newMode;
  lastUltimaticPaddle = 0 ; // to prevent race conditions when switching from Iambic to Ultimatic
}


unsigned long KeyingInterface::getOffTime() { return offTimer; }
unsigned long KeyingInterface::getOnTime() { return onTimer; }