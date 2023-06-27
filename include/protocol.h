#if !defined( PROTOCOL_H )
#define PROTOCOL_H

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
  OnOffEnum serialEcho = OFF ;
  OnOffEnum paddleEcho = OFF ;
  // bool _isStatusDirty = false;
  // word wasResponded = 0;
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
  void service();
};

extern WinkeyProtocol protocol ;

#endif