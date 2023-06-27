#include "keying.h"
#include "paddle.h"
#include "speed_control.h"
// #include "morse.h"
#include "protocol.h"

// debugging
unsigned long blikTime = 0 ;
const byte LED = A0 ;
void blik(bool);

// data type

/* GLOBAL VARIABLES */
KeyingSource keySource = SRC_PADDLE ;
int speedPaddles = 25 ;
int speedBuffer = 20 ;
int speedCommand = 20 ;
unsigned long currentTime ;
unsigned long nextBufferTime;

void setup() {
  currentTime = millis();
  keyer.init();
  keyer.setMode(IAMBIC_B);
  keyer.setTimingParameters(speedPaddles, 300, 50);
  keyer.enableTone(ENABLED);
  paddle.init();
  speedControl->init();
  protocol.init();
  // command mode LED
  pinMode( LED, OUTPUT );
  // debugging
  keyer.setTone(300);
  digitalWrite( LED, HIGH );
  delay(133);
  keyer.setTone(1300);
  delay(133);
  digitalWrite( LED, LOW );
  keyer.setTone(0);
  nextBufferTime = millis();
}

char message[100];
// char qbf[] = "= QUICK BROWN FOX JUMPS OVER THE LAZY DOG.  / 1234567890 + ";
int index = 0 ;

void loop() {
  currentTime = millis();
  speedControl->update();
  int speed = speedControl->getValue();
  if( speed != speedPaddles ) {
    speedPaddles = speed ;
    keyer.setTimingParameters( speedPaddles );
    blik(true);
  }
  else blik(false);
  protocol.service();
  byte paddleState = paddle.check();
  KeyingStatus keyerState = keyer.service( paddleState ) ; // check and update timing and status
  if( keyerState.buffer == ENABLED ) { // can send from buffer?
    keyer.sendCode( protocol.getNextMorseCode() ); // send code or do nothing if got zero
  }
}

// for debugging; remove in production code
void blik(bool start) {
  if( start ) {
    blikTime = currentTime + 25;
    digitalWrite(LED, HIGH);
  }
  else if( blikTime < currentTime ) {
    digitalWrite( LED, LOW );
  }
}
