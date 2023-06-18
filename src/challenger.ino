#include "keying.h"

int phase = 0 ;
void setup() {
  keyingInterface.init();
  keyingInterface.setTimingParameters(18, 500, 20);
  keyingInterface.sendElement( DIT );
}

void loop() {
  KeyingStatus s = keyingInterface.service() ; // check and update timing and status
  ElementType e ;

  if( phase >= 48 ) return ;
  if( s.busy == IDLE || s.nextElement == NO_ELEMENT ) {
    // generate A, R, space, "manually"
    switch (phase++ % 8)
    {
    case 0:
    case 3:
    case 5:
      e = DIT;
      break;
    case 1:
    case 4:
      e = DAH;
      break;
    case 2:
    case 6:
      e = CHARSPACE;
      break;
    case 7:
      e = WORDSPACE;
    }
    keyingInterface.sendElement(e);
  }
}