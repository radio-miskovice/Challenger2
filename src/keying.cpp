#include <Arduino.h>
#include "keying.h"

// Keying interface singleton
KeyingInterface keyingInterface = KeyingInterface() ;

/**
 * initializes Arduino ports
 */
void KeyingInterface::init()
{
  if (_keyline1 >0)  pinMode(_keyline1, OUTPUT);
  if (_pttline1 > 0) pinMode(_pttline1, OUTPUT);
  if (_sidetone > 0) pinMode(_sidetone, OUTPUT);
  if (_cpo_key > 0)  pinMode(_cpo_key, OUTPUT);
}

/**
 * set Key line.
 * @param level - high or low
 */
void KeyingInterface::setKey(KeyerLineStatusEnum onOff)
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
void KeyingInterface::setPtt(KeyerLineStatusEnum onOff)
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

void KeyingInterface::setTiming( byte wpm, word _dahRatio, word _weighting ) {
  unit = 1200 / wpm ;
  dahFactor = (_dahRatio == 0) ? dahFactor : _dahRatio;
  weighting = (_weighting == 0) ? weighting : _weighting;
}

void KeyingInterface::enableKey(KeyerEnableEnum enable)
{
  flags.key  = enable  ; 
  if( flags.key == DISABLED ) setKey(OFF);
}

void KeyingInterface::enablePtt(KeyerEnableEnum enable)
{
  flags.ptt  = enable  ; 
  if( flags.ptt == DISABLED ) setPtt(OFF);
}

void KeyingInterface::enableTone(KeyerEnableEnum enable)
{
  flags.tone = enable ; 
  if( flags.tone == DISABLED ) setTone(OFF);
}

/**
 * This method checks time, services output control timers and updates line levels and sidetone as necessary.
 * @return current service status: IDLE (no timing in progress), BUSY (timing in progress), DIT (sending DIT), DAH (sending dah), SPACE (sending char space or wordspace)
*/
KeyingStatus KeyingInterface::service() {
  if( status.busy == 0 ) return status ; // nothing to do

  unsigned long timestamp = millis();
  unsigned long interval = timestamp - lastMillis ;
  lastMillis = timestamp ;
  if( interval == 0UL ) return status ; // timing in progress, but elapsed zero time, hence no change

  // key on timing
  if( onTimer > 0UL ) {
    if( onTimer < interval ) onTimer = 0 ; 
    else onTimer = onTimer - interval ;
    if( onTimer == 0 ) {
      setKey( OFF );
      setTone( 0 );
    }
    return status ;
  }

  // key off timing 
  if( offTimer > 0 ) {
    if( offTimer < interval ) offTimer = 0 ;
    else offTimer = offTimer - interval ;
    if( offTimer == 0 ) { // element, character or word space expired
      // if no element is waiting in queue, extend space
      if ( status.nextElement == NO_ELEMENT ) {
        switch( status.currentElement ) {
          // when just finished WORDSPACE, nothing else to continue with
          case WORDSPACE: 
            status.busy = IDLE ; // set idle state
            status.currentElement = NO_ELEMENT ; // sending no element
            return status ; // exit right now
            break;

          // when just finished CHARSPACE and no element is waiting, extend to WORDSPACE
          case CHARSPACE:
            status.currentElement = WORDSPACE ;
            offTimer = unit * 4 ; // new offTimer value
            return status ;
            break ;

          default: // this happens only for DIT or DAH
            status.currentElement = CHARSPACE ;
            offTimer = unit * 2 ; // extend element space by 2T to achieve char space 3T
            return status ;
        }
      }
      else {
        newCurrentElement( status.nextElement );
        status.nextElement = NO_ELEMENT ;
        if( status.currentElement == DIT || status.currentElement == DAH ) {
          setKey( ON );
          setTone( toneFreq );
          lastMillis = millis();
        }
      }
    }
  }
  return status ;
}

/***
 * Set element timers and status according to element.
 * @param element new current element to set up
 */
void KeyingInterface::newCurrentElement( ElementType element ) {
  word ditDahFactor = 100 ;   // 100 for DIT, dahFactor for DAH 
  word extraSpaceFactor = 2 ; // 2 for CHARSPACE, 4 for WORDSPACE
  status.currentElement = element ; // set new current element
  status.busy = BUSY ;     // set new status 
  switch( element ) {
    case NO_ELEMENT:
      status.busy = IDLE ;
      onTimer = 0UL ;
      offTimer = 0UL ;
      break ;
    case DAH:
      ditDahFactor = dahFactor ;
    case DIT:
      onTimer = ( unit * weighting ) / 50UL ; // DIT duration with weighting
      offTimer = 2 * unit - onTimer ;  // element space duration with weighting
      // now comes the magic for DAH: multiply by dahFactor, but keep current for DIT
      onTimer = (onTimer * ditDahFactor) / 100UL ;
      break;

    // word space: add 4T pause after 3T character space
    case WORDSPACE:
      extraSpaceFactor = 4 ;
    // charspace: add 2T pause after the last element
    case CHARSPACE:
      onTimer = 0 ;
      offTimer = unit * extraSpaceFactor ;
      break; 
  }
  lastMillis = millis();   // initialize reference timestamp for element timers
}

/**
 * Start sending timed element immediately if the keyer is idle,
 * or put new element in waiting queue otherwise.
*/
KeyingStatus KeyingInterface::sendElement( ElementType element ) {
  // if keyer is IDLE, start new element right off
  if( status.busy == IDLE ) {
    newCurrentElement( element );
    status.nextElement = NO_ELEMENT ;
  }
  // but if the keyer is busy, put the element in queue
  else {
    // we can put it in queue only if the queue is empty
    // but the calling process should never attempt to send 
    // any new element if there is already one waiting!
    if( status.nextElement == NO_ELEMENT ) status.nextElement = element ;
  }
  return status ;
}
