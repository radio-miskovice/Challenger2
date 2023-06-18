#ifndef _PADDLE_H_
#define _PADDLE_H_

#include <Arduino.h>
#include "challenger_types.h"
#include "config_paddle.h"

#if !defined(CONFIG_PADDLE_LEFT) || !defined(CONFIG_PADDLE_RIGHT) || CONFIG_PADDLE_LEFT == 0 || CONFIG_PADDLE_RIGHT == 0
#error "Paddle interface is partially ot fully undefined. Check config_paddle.h"
#endif

enum KeyerModeEnum {
  IAMBIC_A = 0, IAMBIC_B = 1, ULTIMATIC = 2
};

struct PaddleStatus {
  ElementType current: 3 ;
  ElementType next : 3 ;
  KeyerModeEnum mode: 2 ;
  OnOffEnum touch: 1 ;
};

class PaddleInterface {
  
  private:

  static const byte pinPaddleRight = CONFIG_PADDLE_LEFT ;
  static const byte pinPaddleLeft  = CONFIG_PADDLE_RIGHT ;

  PaddleStatus status = { current: NO_ELEMENT, next: NO_ELEMENT, mode: IAMBIC_A, touch: OFF };
  bool swapPaddle = false ;
  byte lastPaddlePortBits = 0 ;
  byte ultimusBits = 0 ;

  public:

  void init() ; // setup ports and initialize variables
  void swap() ;
  PaddleStatus check();   // check paddle status
  PaddleStatus getNext(); // return next element and reset internal state
  PaddleStatus setMode( KeyerModeEnum newMode ); // set new keying mode. Performs reset of all status variables!

};

extern PaddleInterface paddle ; // PaddleInterface singleton instance

#endif