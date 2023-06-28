#include <Arduino.h>
#include "keying.h"

// Keying interface singleton
KeyingInterface keyer = KeyingInterface() ;

void KeyingInterface::enableKey(EnableEnum enable)
{
  flags.key = enable;
  if (flags.key == DISABLED) setKey(OFF);
}

void KeyingInterface::enablePtt(EnableEnum enable)
{
  flags.ptt = enable;
  if (flags.ptt == DISABLED) setPtt(OFF);
}

void KeyingInterface::enableTone(EnableEnum enable)
{
  flags.tone = enable;
  if (flags.tone == DISABLED)  setTone(OFF);
}

void KeyingInterface::setFarnsworthWpm( byte wpm ) {
  farnsWorthWpm = wpm ;
}

/**
 * initializes Arduino ports
 */
void KeyingInterface::init()
{
  if (pin_keyline1 > 0)
    pinMode(pin_keyline1, OUTPUT);
  if (pin_pttline1 > 0)
    pinMode(pin_pttline1, OUTPUT);
  if (pin_sidetone > 0)
    pinMode(pin_sidetone, OUTPUT);
  if (pin_cpo_key > 0)
    pinMode(pin_cpo_key, OUTPUT);
  onTimer = 0UL;
  offTimer = 0UL;
  status.busy = READY;
}

/**
 * Prepare binary morse code for output.
 * Effect:
 *  - ignore morse code 0 (no code, no output)
 *  - if not set, set keying source to SRC_BUFFER
 *  - add character to buffer if the buffer is not full
 * @param code binary code to send
 * @returns {bool} true on success, false otherwise (i.e. when buffer was already full)
 * 
*/
KeyingStatus KeyingInterface::sendCode(byte code)
{
  status.source = SRC_BUFFER;
  if (currentMorse == 0)
    currentMorse = code;
  else
  {
    nextMorse = code;
    status.buffer = DISABLED;
  }
  return status ;
}

/***
 * Set element timers and status according to element.
 * @param element new current element to set up
 */
void KeyingInterface::sendElement(ElementType element)
{
  word elementFactor = 100;   // 100 for DIT, ditDahFactor for DAH
  word extraSpaceFactor = 2;  // 2 for CHARSPACE, 4 for WORDSPACE
  internal.current = element; // set new current element
  status.busy = BUSY;         // set new status
  paddleMemory = PADDLE_FREE;
  switch (element)
  {
  case NO_ELEMENT:
    status.busy = READY;
    onTimer = 0UL;
    offTimer = 0UL;
    setKey(OFF);
    setTone( 0 );
    break;
  case DAH:
    elementFactor = ditDahFactor;
  case DIT:
    onTimer = (unit * weighting) / 50UL; // DIT duration with weighting
    offTimer = 2 * unit - onTimer;       // element space duration with weighting
    // now comes the magic for DAH: multiply by ditDahFactor, but keep current for DIT
    onTimer = (onTimer * elementFactor) / 100UL + qskCompensation ;
    setKey(ON);
    setTone(toneFreq);
    break;

  // word space: add 4T pause after 3T character space
  case WORDSPACE:
    extraSpaceFactor = 4;
  // half space: add 3T
  case HALFSPACE:
  // charspace: add 2T pause after the last element
  case CHARSPACE:
    onTimer = 0;
    offTimer = unit * extraSpaceFactor + (element == HALFSPACE ? 1 : 0);
    setKey(OFF);
    setTone(0);
    break;
  }
  lastMillis = currentTime; // initialize reference currentTime for element timers
}

/**
 * @param input paddle input: bit 0 = DIT (1), bit 1 = DAH (2), value 3 = squeeze
 */
void KeyingInterface::sendPaddleElement(byte input)
{
  ElementType nextElement;
  // for ultimatic code, get unequivocal value of either dot or dash
  // i.e. special handling of squeeze to detect which paddle was added
  if (status.mode == ULTIMATIC)
  {
    input = (paddleMemUltimatic ^ input) & input;
    // at this point {input} is one of DOT, DAH, PADDLE_FREE
    paddleMemUltimatic = input;
  }
  // in case of IAMBIC B, substitute no paddle contact by paddle memory
  else if (status.mode == IAMBIC_B && input == PADDLE_FREE) // use memory in case of Iambic B status.mode
  {
    input = paddleMemory;
    paddleMemory = PADDLE_FREE;
  }
  // at this point we have final "paddle value"
  // next element is determined accordingly
  switch (input)
  {
  case 3: // squeeze; after previous transformation, ultimatic will never have 3
    nextElement = (internal.last == DIT) ? DAH : DIT;
    break;
  case 0:
    nextElement = NO_ELEMENT;
    break; // NO_ELEMENT is already in NextElement, which would be the "else" branch
  default: // the rest is either DIT or DAH
    nextElement = (ElementType)input;
  }
  sendElement(nextElement);
}

void KeyingInterface::setFirstExtension(byte ms)
{
  if( ms <= 250 ) firstExtension = ms;
}

void KeyingInterface::setQskCompensation(byte ms)
{
  if (ms <= 250 ) qskCompensation = ms;
}

/**
 * set Key line.
 * @param level - high or low
 */
void KeyingInterface::setKey(OnOffEnum onOff)
{
  if( flags.key == ENABLED ) {
    digitalWrite(pin_keyline1, onOff);
    status.key = onOff;
  } 
  else {
    digitalWrite(pin_keyline1, LOW);
    status.key = OFF ;
  }
}

/**
 * set Key line immediately with timeout. Used for tuning transciever etc.
 * @param level - high or low
 */
void KeyingInterface::setKey(OnOffEnum onOff, word timeout)
{
  if (flags.key == ENABLED || timeout > 0)
  {
    digitalWrite(pin_keyline1, onOff);
    status.key = onOff;
    status.force = ON ;
    hardKeyTimeout = currentTime + timeout ;
  }
  else
  {
    digitalWrite(pin_keyline1, LOW);
    status.key = OFF;
    status.force = OFF ;
  }
}

/**
 * Set new status.mode
 */
void KeyingInterface::setMode(KeyerMode newMode)
{
  status.mode = newMode;
  paddleMemUltimatic = 0; // to prevent race conditions when switching from Iambic to Ultimatic
  paddleMemory = 0;
}

/**
 * @param level - high or low
*/
void KeyingInterface::setPtt(OnOffEnum onOff)
{
  if( flags.ptt == ENABLED ) {
    digitalWrite(pin_pttline1, onOff);
    status.ptt = onOff ;
  }
  else {
    digitalWrite(pin_pttline1, LOW);
    status.ptt = OFF ;
  }
}

void KeyingInterface::setPttTiming( byte lead, byte tail ) {
  pttLead = lead; pttTail = tail ; 
}
/**
 * @param hz when hz = 0 stops sidetone, otherwise starts sidetone with frequency hz Hz
*/
void KeyingInterface::setTone(word hz)
{
  hz = trimToneFreq(hz) ;
  if( hz > 0)
    tone(pin_sidetone, hz); 
  else noTone(pin_sidetone);
}

/**
 * @param hz when hz = 0 stops sidetone, otherwise starts sidetone with frequency hz Hz
 */
void KeyingInterface::setSource(KeyingSource s)
{
  status.source = s ;
}


/**
 *  @param  hz sidetone frequency to set for high-level sending
 */
void KeyingInterface::setToneFreq( word hz ) {
  hz = trimToneFreq(hz);
  if( hz > 0 ) toneFreq = hz ;
}

void KeyingInterface::setTimingParameters( byte wpm, word _dahRatio, word _weighting ) {
  unit = (wpm > 5) ? 1200 / wpm : unit ;
  ditDahFactor = (_dahRatio == 0) ? ditDahFactor : _dahRatio;
  weighting = (_weighting == 0) ? weighting : _weighting;
}

/**
 * This method checks time, services output control timers and updates line levels and sidetone as necessary.
 * @return current service status: READY (no timing in progress), BUSY (timing in progress), DIT (sending DIT), DAH (sending dah), SPACE (sending char space or wordspace)
*/
KeyingStatus KeyingInterface::service( byte paddleState ) {
  // check current time 
  unsigned long interval = currentTime - lastMillis ; 
  // (1) check paddle break and hard keydown timeout. Breaks buffer send and forced keydown.
  bool breakInFlag =  paddleState > 0 && (status.source == SRC_BUFFER && status.busy == BUSY);
  bool forceTimeoutFlag = (hardKeyTimeout <= currentTime && status.force == ON); 
  if ( breakInFlag || forceTimeoutFlag || (status.force == ON && paddleState > 0))
  {
    // common for all breaks:
    // stop sending:
    setKey(OFF);
    setTone(0);
    // clear element buffers & timer
    internal.current = NO_ELEMENT ;
    internal.last = NO_ELEMENT ;
    onTimer = 0 ;
    status.force = OFF ;
    offTimer = unit ; // leave 1T to handle paddle break in the main loop
    // Buffer specific:
    if( status.source == SRC_BUFFER ) {
      // clear morse codes
      currentMorse = 0;
      nextMorse = 0;
      // set break-in status, it has to be reported to protocol
      status.breakIn = ON ;
      status.buffer = DISABLED ; // do not accept further codes until breakIn is cleared
    }
    return status ; // paddle break event is the last action in this tick if occured
  }
  // otherwise check scheduled actions - first do current element
  if( interval == 0UL ) return status ; // timing in progress, but elapsed zero time, hence no change
  lastMillis = currentTime ; // remember new current time
  // (2) service KEY DOWN state
  if( onTimer > 0UL ) {
    if (onTimer < interval) onTimer = 0;
    else onTimer = onTimer - interval;
    if (onTimer == 0) { // KEY DOWN just finished:
      setKey(OFF);                // switch off key line
      setTone(0);                 // switch off sidetone
      paddleMemory = PADDLE_FREE; // clear paddle memory for Iambic B
    }
    return status; // element in progress, no other action is possible
  }
  // (3) service KEY UP state
  if (offTimer > 0) {
    paddleMemory = paddleMemory | paddleState; // record paddle state for Iambic B
    if (offTimer < interval) offTimer = 0;
    else offTimer = offTimer - interval;
    if (offTimer == 0) { // if just finished pause
      internal.last = internal.current;  // record completed element to memory
      internal.current = NO_ELEMENT;  // currently have no other element
      status.busy = READY; // this is true if nothing else happens, see below
      // clear break-in status if still active
      if( status.breakIn == ON ) {
        status.breakIn = OFF ;
        status.buffer = ENABLED ;
        status.source = SRC_PADDLE ;
      }
    }
    else { return status ; } // if pause is in progress, no more actions follow
  }
  // (4) service buffered morse code 
  // The section above just finished element pause, so serve next element
  if (status.source == SRC_BUFFER && status.busy == READY) {
    if (currentMorse == 0) { // current code has finished
      if (nextMorse != 0) { // fetch next
        currentMorse = nextMorse;
        nextMorse = 0; // clear FIFO
        status.buffer = ENABLED;
      }
      // otherwise switch to paddle mode if no more codes in buffer
      else { 
        status.source = SRC_PADDLE; 
        sendElement(NO_ELEMENT);
      } // switch to paddle if no more morse codes
    }
    // now handle non-empty morse codes
    switch (currentMorse) {
      // case 0: break; // this is redundant, handled above!
      case MORSE_SPACE :
        sendElement( WORDSPACE );
        currentMorse = 0 ; // remove the explicit space code
        break ;
      case MORSE_CHARSPACE:
        sendElement( CHARSPACE ); // same as above, but shorter
        currentMorse = 0 ;
        break ;
      default:
        ElementType e = ((currentMorse & 0x80) == 0) ? DIT : DAH;
        sendElement(e); // prepare next element and continue to timing section
    }
    currentMorse <<= 1; // shift to next element
  }
  if( status.source == SRC_BUFFER ) return status ; // if sending buffer, we don't check paddles
  // (5) last action: check paddles and play element if paddles pressed
  // as a result of previous actions, at this point status must be READY
  // and source must be PADDLE
  sendPaddleElement(paddleState);
  return status ; // always return status to allow for proper interaction with other components
}

/**
 * @param hz required sidetone frequency
 * @returns sidetone frequency trimmed to remain between limits
 */
word KeyingInterface::trimToneFreq(word hz)
{
  if (hz == 0)
      return 0;
  return (hz < minToneFreq ? minToneFreq : (hz > maxToneFreq ? maxToneFreq : hz));
}
