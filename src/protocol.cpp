// #include <Arduino.h>
#include "config_protocol.h"
#include "morse.h"
#include "keying.h"
#include "paddle.h"
#include "speed_control.h"
#include "protocol.h"

const word WINKEY_SIDETONE_FREQ = 4000;

const byte BUFFER_XOFF_LIMIT = 4;
const byte BUFFER_XON_LIMIT = 16;

const byte WKS_READY    = 0xC0 ; // send to report everything OK and also after WKS_BREAKIN (to make N1MM happy)
const byte WKS_BUFFERED = 0xC4 ; // send when accepted first character to buffer
const byte WKS_XOFF     = 0xC5 ; // send when buffer almost full (fifo.getFree() < BUFFER_XOFF_LIMIT )
const byte WKS_XON      = 0xC4 ; // send when in XOFF condition and fifo.getFree() > BUFFER_XON_LIMIT
const byte WKS_BREAKIN  = 0xC6 ; // send on paddle break-in event (must be followed by 0xC0)

/*
 * Jumping to 0x0000 will restart the whole program
 */
void reboot_cpu() {
  void (*reboot)() = 0x0000;
  (*reboot)();
}

// Parameter size table for Winkeyer commands. 
// for regular commands 0x01 through to 0x1F: index = command code. Index zero is not valid.
// for admin commands, offset is 0x20, i.e. [0x20] => command <0> <0>
const byte parametersExpected[] = {
  0, 1, 1, 1,  // n/a, sidetone, wpm, weighting
  2, 3, 1, 0,  // ptt delays, pot range, pause morse, request pot value
  0, 1, 0, 1,  // backup input pointer, set output pin, clear buffer, key immediate
  1, 1, 1, 15, // hscw speed, farnswth speed, wk2 mode, load defaults

  1, 1, 1, 0,  // 1st ext, key comp, paddle sensitivity, nop
  1, 0, 1, 1,  // sw paddle, req status, buffer ptr, dah/dit ratio
  1, 1, 1, 2,  // buff ptt, buff keydown, buff wait, merge prosign
  1, 1, 0, 0,   // buff wpm, buff hscw or buff port sel, buff reset speed, buff nop

  // admin commands (prefix 0x00, table offset 0x20)

  3, 0, 0, 0,  // calibrate, reset, host open, host close
  1, 0, 0, 0,  // echo, -, -, get values
  0, 0, 0, 0,
  255, 1
};

WinkeyProtocol protocol ; // protocol singleton


/**
 * Execute command fetched in command buffer
 */
void WinkeyProtocol::executeCommand()
{
  if (phase != EXECUTE)
    return;
  // TODO: execute command
  switch (command)
  {
  case 0x24:                // Admin: Send Echo - param[0] contains character to be echoed
  case 0x25:                // Admin: Paddle A2D
  case 0x26:                // Admin: Speed A2D
  case 0x29:                // Admin: Get Calibration
    sendResponse(param[0]); // param[0] contains zero in commands with no params
    break;
  // Reset
  case 0x21:
    reboot_cpu();
    break;
  // Host Open
  case 0x22:
    _isHostOpen = true;
    sendResponse(WK_REVISION);
    break;
  // Host Close
  case 0x23:
    _isHostOpen = false;
    break;
  // Status Mode WK1
  case 0x2A:
    wkStatusMode = WK1;
    break;
  // Status Mode WK2
  case 0x2B:
    wkStatusMode = WK2;
    break;
  // Sidetone Control
  case 0x01:
    _sidetonePaddleOnly = (param[0] & 0x80) != 0;
    param[0] = param[0] & 0x0F;
    if (param[0] != 0)
    {
      keyer.setToneFreq(4000 / param[0]);
    }
    break;
  case 0x02: // set WPM
    keyer.setTimingParameters(param[0]);
    break;
  case 0x03: // set weighting
    keyer.setTimingParameters(0, 0, param[0]);
    break;
  case 0x04: // set PTT head, tail
    keyer.setPttTiming( param[0], param[1]);
    break;
  case 0x05: // set pot range
    speedControl->setMinMax(param[0], param[0] + param[1]);
    break;
  case 0x07: // get pot value
    sendResponse(speedControl->getSpeedWk2());
    break;
  case 0x0A:
    fifo.reset();
    break;
  case 0x0B:
    keyer.setKey(ON, 15000);
    break;
  case 0x0D:
    keyer.setFarnsworthWpm(param[0]);
    break;
  case 0x0E:
    setModeParameters();
    break;
  // all unimplemented commands w/o response
  case 0x0F: // load defaults...
    // TODO: load defaults
    setModeParameters(); // implicit param[0]
    keyer.setTimingParameters(param[1], (param[12] * 300U) / 50U, param[3]) ;
    keyer.setPttTiming(param[4], param[5]);
    keyer.setFirstExtension( param[8] );
    keyer.setQskCompensation( param[9]);
    speedControl->setValue(param[1]);
    speedControl->setMinMax(param[6], param[6] + param[7]);
    keyer.setFarnsworthWpm(param[10]);
    break;
  case 0x10: // 1st extension
    keyer.setFirstExtension( param[0] );
    break;
  case 0x11: // QSK compensation 
    keyer.setQskCompensation( param[0] );
    break;
  case 0x15: // Winkeyer2 status
    sendStatus();
    break;
  case 0x17: // dah:dit ratio
    keyer.setTimingParameters(0, (param[0] * 300U) / 50U, 0);
    break;
  // buffered commands
  case 0x18: // buffered PTT
    // fifo.push(command + 0xF0 * param[0]);
    break;
  default:
    ignore();
  }
  phase = FETCH_ANY;
}

/**
 * @return {byte} morse code of the next character from buffer, or zero if nothing to send
**/
byte WinkeyProtocol::getNextMorseCode() {
  byte c = 0 ;
  if( fifo.hasMore() ) {
    c = fifo.shift() ; // returns zero if buffer is empty
    if( c ) {
      c = morse.asciiToCode( c );
    }
    if( fifo.getLength() == 0 ) sendStatus( WKS_READY ); // send READY if buffer is empty
    else {
      if( bufferFull && fifo.getFree() > BUFFER_XON_LIMIT ) sendStatus( WKS_XON ); // send XON if buffer was partially freed
      else sendStatus( WKS_BUFFERED );
    }
  }
  return c ; // return morse code from buffer or zero if no code
}

void WinkeyProtocol::ignore() {}

void WinkeyProtocol::init() {
  Serial.begin( SERIAL_SPEED ); // 1k2 is the only winkeyer protocol baud rate
  fifo.reset();
  phase = FETCH_ANY ;
}
/**
 * @returns {bool} true if host is open, false otherwise
 */
bool WinkeyProtocol::isHostOpen() { return _isHostOpen; }

void WinkeyProtocol::sendResponse( byte x ) {
  Serial.write(x);
}

// void WinkeyProtocol::sendResponse(word x)
// {
//   Serial.write((byte)(x & 0xFF));
//   x >>= 8;
//   Serial.write((byte)(x & 0xFF));
// }

void WinkeyProtocol::sendResponse(char* str, word length)
{ Serial.write( str, length ); }

/**
 * Send Winkeyer2 status byte explicit value
 */
void WinkeyProtocol::sendStatus(void)
{
  sendResponse(lastWkStatus);
}

/**
 * Send Winkeyer2 status byte explicit value, only if it differs from previous status
 */
void WinkeyProtocol::sendStatus( byte status ) {
  if( status != lastWkStatus ) {
    sendResponse( status );
    lastWkStatus = status ;
  }
}

/**
 * Send Winkeyer2 status byte from KeyerState
 */
void WinkeyProtocol::sendStatus( KeyerState kState )
{ 
  byte wks = WKS_READY ;
  if( kState.source == SRC_BUFFER ) wks |= WKS_BUFFERED ;
  if( kState.breakIn == ON ) wks |= WKS_BREAKIN ;
  if( bufferFull ) wks |= WKS_XOFF ;
  sendStatus( wks );
}
/**
 * Reads serial port if character is available, except if previous character is still waiting to be processed.
 * Command characters are taken immediately into command buffer. All other characters are sent to circular text buffer
 * or stored temporarily in pendingChar in case of buffer congestion.
 */
void WinkeyProtocol::service()
{
  int input;
  input = Serial.peek() ;
  while (input >= 0 
         && (phase == EXPECT_ADMIN || phase == EXPECT_PARAMS 
             || (phase == FETCH_ANY && (input <= 0x1F || fifo.canTake()) ))
        )
  {
    // read one character
    input = Serial.read();
    switch (phase)
    {
    case FETCH_ANY:
      if( input == 0 ) {
        phase = EXPECT_ADMIN ;
      }
      else if( input < 0x20 ) {
        command = input;
        bytesExpected = parametersExpected[command];
        if (bytesExpected == 255) bytesExpected++; // only for donwload EEPROM command
        bytesFetched = 0;
        if (bytesExpected > 0) phase = EXPECT_PARAMS;
        else {
          phase = EXECUTE;
          param[0] = 0;
        }
      }
      else {
        fifo.push(input);
        if( echo.serial == ON ) Serial.write( (char) input ); // do serial echo 
        if( fifo.getFree() <= BUFFER_XOFF_LIMIT ) sendStatus( WKS_XOFF );
      }
      break;
    case EXPECT_ADMIN:
      command = 0x20 + input; // complete command code
      if (command > 0x2D)
      { // invalid admin command code?
        phase = FETCH_ANY;
      }
      else
      {
        bytesExpected = parametersExpected[command];
        bytesFetched = 0;
        if (bytesExpected > 0) phase = EXPECT_PARAMS;
        else {
          phase = EXECUTE;
          param[0] = 0;
        }
      }
      break;
    case EXPECT_PARAMS:
      if (bytesExpected > 0)
      {
        if (bytesFetched < 16)  param[bytesFetched] = input; // ignore bytes after 16th byte, this is part of ignoring EEPROM download
        bytesFetched++;
        if (command == 0x16 && bytesFetched == 1 && input == 3) bytesExpected++; // command Buffer Pointer Command has extra byte if parameter == 3
        bytesExpected--;
      }
      if (bytesExpected == 0) phase = EXECUTE;
    default:
      break; 
    }
    input = Serial.peek();
  }
  if (phase == EXECUTE) executeCommand();
}

void WinkeyProtocol::setModeParameters()  { 
  byte wkMode = param[0] ;
  // TODO: paddle watchdog (implement in KeyingInterface)
  // keyer.setPaddleWatchdog( wkMode & 0x80 )
  // TODO: paddle echo (implement in KeyingInterface)
  // keyer.setPaddleEcho( wkMode & 0x40 )
  // key mode
  switch( wkMode & 0x30 ) {
    case 0: keyer.setMode( IAMBIC_B); break ;
    case 0x10: keyer.setMode(IAMBIC_A); break ;
    case 0x20: keyer.setMode(ULTIMATIC); break ;
    default: ;
  }
  if( wkMode & 8 ) paddle.swap();
  echo.serial = (wkMode & 4) ? ON : OFF ;
  echo.paddle = (wkMode & 0x40) ? ON : OFF ;
  // TODO: autospace (implement in KeyingInterface)
  keyer.setAutospace( (wkMode & 2) ? ENABLED : DISABLED );
  // TODO: contest spacing (implement in KeyingInterface)
  // keyer.setContestSpacing( wkMode & 1 )
}

// void WinkeyProtocol::setStatus(KeyerStateWord keyState) {
//   byte status = wkStatusFromKeyerState( keyState ) ;
//   statusChanged = statusChanged || (status != wkStatus) ;
//   if(statusChanged) {
//     wkStatus = status ;
//   }
// }

/**
 * event handler for paddle break-in
**/
void WinkeyProtocol::stopBuffer() {
  fifo.reset();
  sendStatus( WKS_BREAKIN );
  sendStatus( WKS_READY ); 
}
