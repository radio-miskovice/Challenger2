#if !defined( _PROTOCOL_H_ )
#define _PROTOCOL_H_

#include "challenger.h"
#include "keying.h"
#include "buffer.h"

enum FetchProgressPhase : byte
{
  FETCH_ANY,
  EXPECT_ADMIN,
  EXPECT_PARAMS,
  EXECUTE
};

enum WinkeyStatusMode : byte
{
  WK1,
  WK2
};

struct EchoFlags {
  OnOffEnum serial: 1;
  OnOffEnum paddle: 1;
};

class WinkeyProtocol
{
private:
  static const byte WK_REVISION = 22; // Winkey protocol revision number
  WinkeyStatusMode wkStatusMode = WK1; // Winkey mode; ignored
  // serial port reading variables
  FetchProgressPhase phase = FETCH_ANY;
  word bytesExpected;    // total bytes to be read into param
  word bytesFetched = 0; // total bytes fetched into param
  byte command;          // command byte, prefix = 0x20 for admin commands
  byte param[16];        // command parameter(s)
  // status variables
  byte lastWkStatus = 0xC0 ; // remember last status sent
  byte _isHostOpen;      // host status; ignored
  bool _sidetonePaddleOnly = false; // unused?
  bool bufferFull = false ; // flag indicating that XOFF was reported to host
  EchoFlags echo = { serial: ON, paddle: OFF };
  CharacterFIFO fifo; // text buffer 256 bytes
  void ignore(); // method to handle ignored WK commands
  void setModeParameters();
  byte wkStatusFromKeyerState( KeyerState ks );
  // debugging message
  char message[80];

public:
  // bool expectCmd = false;
  void executeCommand();
  byte getNextMorseCode();
  void init();
  bool isHostOpen();
  void sendPaddleEcho(byte ascii);
  void sendResponse(byte);
  // void sendResponse(word);
  void sendResponse(char* str, word length);
  void sendStatus();
  void sendStatus( byte wkStatus );
  void sendStatus( KeyerState keyState );
  void service();
  // void setStatus( KeyerStateWord keyState );
  void stopBuffer() ; 
  // testing only, M7
  void enablePaddleEcho(OnOffEnum e);
};

extern WinkeyProtocol protocol ;

#endif