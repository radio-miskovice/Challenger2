#include "keying.h"
#include "paddle.h"
#include "speed_control.h"


// debugging
void blik(bool);
unsigned long blikTime = 0 ;

// data type

/* GLOBAL VARIABLES */
KeyingSource keySource = SRC_PADDLE ;
int speedPaddles = 25 ;
int speedBuffer = 20 ;
int speedCommand = 20 ;

void setup() {
  pinMode( LED_BUILTIN, OUTPUT );
  keyer.init();
  keyer.setMode(IAMBIC_B);
  keyer.setTimingParameters(speedPaddles, 300, 50);
  paddle.init();
  speedControl->init();
}

void loop() {
  speedControl->update();
  int speed = speedControl->getValue();
  if( speed != speedPaddles ) {
    speedPaddles = speed ;
    keyer.setTimingParameters( speedPaddles );
    blik(true);
  }
  else blik(false);
  KeyingStatus ks = keyer.service() ; // check and update timing and status
  if( ks.busy == IDLE ) {
    byte ps = paddle.check() ;
    keyer.sendPaddleElement(ps);
  }
}

// for debugging; remove in production code
void blik(bool start) {
  if( start ) {
    blikTime = millis() + 25;
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if( blikTime < millis()) {
    digitalWrite( LED_BUILTIN, LOW );
  }
}