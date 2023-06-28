#include <Arduino.h>
#include "morse.h"
#include "keying.h"
#include "paddle.h"
#include "speed_control.h"
#include "protocol.h"

const word WINKEY_SIDETONE_FREQ = 4000;
const byte WKS_TEMPLATE = 0b11000000; // Winkey status marker
const byte WKS_WAIT     = 0b00010000; // Winkey "wait" <=> keyer.busy
const byte WKS_KEYDOWN  = 0b00001000; // Winkey "keydown" <=> keyer.force & keyer.key; only WK1 mode
const byte WKS_BUSY     = 0b00000100; // Winkey "sending morse" <=> keyer.source == buffer
const byte WKS_BREAKIN  = 0b00000010; // Winkey "paddle break-in" <=>
const byte WKS_XOFF     = 0b00000001; // Winkey "XOFF" when buffer more than 2/3 full (170+ characters) 

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
    statusChanged = true ; // status will be reported after update
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

byte WinkeyProtocol::getNextMorseCode() {
  if( fifo.hasMore() ) {
    byte c = fifo.shift() ;
    return 0b01010000 ;
    if( c ) {
      c = morse.asciiToCode( c );
      return c ;
    }
  }
  return 0 ;
}

void WinkeyProtocol::ignore() {}

void WinkeyProtocol::init() {
  Serial.begin( 1200 ); // 1k2 is the only winkeyer protocol baud rate
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

void WinkeyProtocol::sendResponse(word x)
{
  Serial.write((byte)(x & 0xFF));
  x >>= 8;
  Serial.write((byte)(x & 0xFF));
}

void WinkeyProtocol::sendResponse(char* str, word length)
{ Serial.write( str, length ); }
void WinkeyProtocol::sendStatus() {
  if( statusChanged ) Serial.write( wkStatus );
  statusChanged = false ; 
}
/**
 * Reads serial port if character is available, except if previous character is still waiting to be processed.
 * Command characters are taken immediately into command buffer. All other characters are sent to circular text buffer
 * or stored temporarily in pendingChar in case of buffer congestion.
 */
void WinkeyProtocol::service()
{
  int input;
  // repeat as long as we can
  input = Serial.peek() ;
  while (input >= 0 
         && ((phase == FETCH_ANY && input < 0x20) || (phase == FETCH_ANY && fifo.canTake() )  
              || phase == EXPECT_ADMIN || phase == EXPECT_PARAMS))
  {
    // read one character
    input = Serial.read();
    switch (phase)
    {
    case FETCH_ANY:
      if( (input >= ' ' && input <= 'Z') || (input >= 'a' && input <= 'z') || input == '|' ) {
        fifo.push( input );
        Serial.write( input );
        Serial.println();
        Serial.println( fifo.getFree() );
      }
      // if the byte is below 0x20, start fetching command
      else if (input <= 0x1F)
      {
        command = input; // copy byte into word
        if (input == 0)
          phase = EXPECT_ADMIN;
        else
        {
          command = input;
          bytesExpected = parametersExpected[command];
          if (bytesExpected == 255)
            bytesExpected++; // only for donwload EEPROM command
          bytesFetched = 0;
          if (bytesExpected > 0)
            phase = EXPECT_PARAMS;
          else
          {
            phase = EXECUTE;
            param[0] = 0;
          }
        }
      }
      // otherwise push it to text buffer
      else
        fifo.push(input);
      break;
    case EXPECT_ADMIN:
      command = 0x20 + input; // complete command code
      if (command > 0x2D)
      { // invalid admin command code?
        phase = FETCH_ANY;
      }
      else
      {
        bytesFetched = 0;
        bytesExpected = parametersExpected[command];
        if (bytesExpected > 0)
          phase = EXPECT_PARAMS;
        else
        {
          phase = EXECUTE;
          param[0] = 0;
        }
      }
      break;
    case EXPECT_PARAMS:
      if (bytesExpected > 0)
      {
        if (bytesFetched < 16)
          param[bytesFetched] = input; // ignore bytes after 16th byte, this is part of ignoring EEPROM download
        bytesFetched++;
        if (command == 0x16 && bytesFetched == 1 && input == 3)
          bytesExpected++; // command Buffer Pointer Command has extra byte if parameter == 3
        bytesExpected--;
      }
      if (bytesExpected == 0)
        phase = EXECUTE;
    default:
      break; 
    }
  }
  if( fifo.getLength() == 0 ) bufferBreak = false ;
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
  serialEcho = (wkMode & 4) ? ON : OFF ;
  // TODO: autospace (implement in KeyingInterface)
  // keyer.setAutospace( wkMode & 2 )
  // TODO: contest spacing (implement in KeyingInterface)
  // keyer.setContestSpacing( wkMode & 1 )
}

void WinkeyProtocol::setStatus(KeyingStatus keyState, byte paddleState) {
  byte status = WKS_TEMPLATE ;
  if( keyState.busy == BUSY ) status |= WKS_WAIT ;
  if( keyState.source == SRC_BUFFER ) status |= WKS_BUSY ;
  if( bufferBreak ) status |= WKS_BREAKIN ;
  if( fifo.getLength() > 170 ) status |= WKS_XOFF ;
  statusChanged = statusChanged || (status != wkStatus) ;
  wkStatus = status ;
}

void WinkeyProtocol::stopBuffer() {
  fifo.reset();
  bufferBreak = true ;
}