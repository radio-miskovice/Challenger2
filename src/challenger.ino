#include "keying.h"
#include "paddle.h"

void setup() {
  keyer.init();
  paddle.init();
  keyer.setTimingParameters(25, 300, 50);
}

void loop() {
  KeyingStatus ks = keyer.service() ; // check and update timing and status
  if( ks.busy == IDLE ) {
    byte ps = paddle.check() ;
    switch (ps) {
      case 0: keyer.sendElement( NO_ELEMENT ) ; break ;
      case 3: keyer.sendElement( ks.last == DIT ? DAH : DIT ); break ;
      default: keyer.sendElement( (ElementType) ps );
    }
  }
}