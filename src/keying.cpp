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
  status.breakIn = OFF ;
  paddleMemory = PADDLE_FREE;
  switch (element)
  {
  case NO_ELEMENT:
    status.busy = READY;
    onTimer = 0UL;
    offTimer = 0UL;
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
  // if playing text buffer and paddle was touched, break and add a short pause; NO KEYING!
  if (status.source == SRC_BUFFER && input != PADDLE_FREE)
  {
    status.source = SRC_PADDLE; // switch to paddle status.mode
    sendElement(CHARSPACE);
    return;
  }
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

void KeyingInterface::setBreakIn() {
  status.breakIn = OFF;
  status.source = SRC_PADDLE ;
  status.buffer = ENABLED ;
  internal.last = NO_ELEMENT;
  internal.current = NO_ELEMENT;
  paddleMemUltimatic = PADDLE_FREE;
  paddleMemory = PADDLE_FREE;
  currentMorse = 0 ;
  nextMorse = 0 ;
  setKey(OFF, 0);
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
  if (flags.key == ENABLED)
  {
    digitalWrite(pin_keyline1, onOff);
    status.key = onOff;
    status.force = ON ;
    hardKeyTimeout = currentTime + timeout ;
  }
  else
  {
    digitalWrite(pin_keyline1, LOW);
    setTone(0);
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
  unit = 1200 / wpm ;
  ditDahFactor = (_dahRatio == 0) ? ditDahFactor : _dahRatio;
  weighting = (_weighting == 0) ? weighting : _weighting;
}

/**
 * This method checks time, services output control timers and updates line levels and sidetone as necessary.
 * @return current service status: READY (no timing in progress), BUSY (timing in progress), DIT (sending DIT), DAH (sending dah), SPACE (sending char space or wordspace)
*/
KeyingStatus KeyingInterface::service( byte ps ) {
  unsigned long interval = currentTime - lastMillis ;
  // if sending from paddle, check paddles if ready to send next
  if( status.source == SRC_PADDLE && status.busy == READY ) {
    sendPaddleElement( ps );
    status.breakIn = OFF ;
    return status ;
  }
  // stop forced key down on timeout or paddle touch or buffer character
  if( status.force ) {
    if( ps > 0 || hardKeyTimeout <= currentTime || status.source != SRC_PADDLE ) {
      setBreakIn();
    }
  }
  // when playing buffer and paddles are touched, act immediately
  if( status.source == SRC_BUFFER && ps > 0 ) {
    // paddle is touched while playing buffer => STOP buffer
    // TODO: test and replace by more adequate timing
    sendElement( CHARSPACE ); // ensure key off, give time to settle
    setBreakIn(); // key off, sidetone off, buffer reset, paddle memory reset, set breakin condition
  }
  // after immediate actions check scheduled actions
  if( interval == 0UL ) return status ; // timing in progress, but elapsed zero time, hence no change
  lastMillis = currentTime ; // remember new current time
  // service buffered morse code 
  if (status.source == SRC_BUFFER && status.busy == READY)
  {
    if (currentMorse == 0) { // current code is finished
      if (nextMorse != 0) { // fetch next
        currentMorse = nextMorse;
        nextMorse = 0; // clear FIFO
        status.buffer = ENABLED;
      }
      // otherwise switch to paddle mode if no more codes in buffer
      else { status.source = SRC_PADDLE; }
    }
    // now handle non-empty morse codes
    switch (currentMorse) {
      case 0: break;
      case MORSE_SPACE :
        sendElement( WORDSPACE );
        currentMorse = 0 ; // remove the explicit space code
        break ;
      case MORSE_CHARSPACE:
        sendElement( CHARSPACE );
        break ;
      default:
        ElementType e = ((currentMorse & 0x80) == 0) ? DIT : DAH;
        sendElement(e); // prepare next element and continue to timing section
    }
    currentMorse <<= 1; // shift to next element
  }

  // timing section for key=ON
  if( onTimer > 0UL ) {
    if( onTimer < interval ) onTimer = 0 ; 
    else onTimer = onTimer - interval ;
    if( onTimer == 0 ) {
      setKey( OFF ); // switch off key line
      setTone( 0 );  // switch off sidetone
      paddleMemory = PADDLE_FREE ; // clear paddle memory for Iambic B
    }
    return status ;
  }

  // timing section for key=OFF 
  if( offTimer > 0 ) {
    paddleMemory = paddleMemory | ps ;
    if( offTimer < interval ) offTimer = 0 ;
    else offTimer = offTimer - interval ;
    if( offTimer == 0 ) { 
       internal.last = internal.current ;
       internal.current = NO_ELEMENT ;
       status.busy = READY ;
      }
    }
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
