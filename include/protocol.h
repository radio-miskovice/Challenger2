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

class WinkeyProtocol
{
private:
  static const byte WK_REVISION = 22; // Winkey protocol revision number
  FetchProgressPhase phase = FETCH_ANY;
  WinkeyStatusMode wkStatusMode = WK1;
  word bytesExpected;    // total bytes to be read into param
  word bytesFetched = 0; // total bytes fetched into param
  byte command;          // command byte, prefix = MSB
  byte param[16];        // command parameter(s)
  byte _isHostOpen;
  byte wkStatus ;
  bool bufferBreak = false ;
  bool statusChanged = true ;
  OnOffEnum serialEcho = OFF ;
  OnOffEnum paddleEcho = OFF ;
  bool _sidetonePaddleOnly = false;
  CharacterFIFO fifo;
  void ignore();
  void setModeParameters();

public:
  // bool expectCmd = false;
  void executeCommand();
  byte getNextMorseCode();
  void init();
  bool isHostOpen();
  void sendResponse(byte);
  void sendResponse(word);
  void sendResponse(char* str, word length);
  void sendStatus();
  void service();
  void setStatus( KeyingStatus keyState, byte paddleState );
  void stopBuffer() ; 
};

extern WinkeyProtocol protocol ;

#endif