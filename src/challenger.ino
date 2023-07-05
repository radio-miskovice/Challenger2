#include "keying.h"
#include "paddle.h"
#include "speed_control.h"
#include "protocol.h"

// debugging
unsigned long blikTime = 0 ;
const byte LED = CONFIG_CMD_MODE_LED ;
void blik(bool);

// data type

/* GLOBAL VARIABLES */
KeyingSource keySource = SRC_PADDLE ;
int speedPaddles = 25 ;
int speedBuffer = 20 ;
int speedCommand = 20 ;
unsigned long currentTime ;

void setup() {
  pinMode( LED_BUILTIN, OUTPUT );
  digitalWrite( LED_BUILTIN, HIGH );
  protocol.init();
  keyer.init();
  keyer.setMode(IAMBIC_B);
  keyer.setTimingParameters(speedPaddles, 300, 50);
  keyer.enableTone(ENABLED);
  paddle.init();
  speedControl->init();
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
  delay(100);
  currentTime = millis();
  keyer.service(0);
  keyer.sendCode(0b10101000);
  keyer.sendCode(0b00111100);
  speedControl->setMinMax(15,46);
  speedControl->setValue(speedPaddles);
  digitalWrite( LED_BUILTIN, LOW );
}

char message[100];
// char qbf[] = "= QUICK BROWN FOX JUMPS OVER THE LAZY DOG.  / 1234567890 + ";
int index = 0 ;
KeyerState kkk ;

void loop() {
  currentTime = millis();
  speedControl->update();
  int speed = speedControl->getValue();
  if( speed != speedPaddles ) {
    speedPaddles = speed ;
    keyer.setTimingParameters( speedPaddles );
    blik(true);
    protocol.sendResponse( speedControl->getSpeedWk2() ); // send WK status speed info
  }
  else blik(false);
  protocol.service(); // check incoming data and execute command if necessary
  byte paddleState = paddle.check();
  KeyerState keyerState = keyer.service( paddleState ) ; // check and update timing and status
  if( keyerState.breakIn == ON ) {
    protocol.stopBuffer(); // will also send WK status "breakin" and "ready"
  }
  if( keyer.canAccept() ) { // can send from buffer?
     byte x = protocol.getNextMorseCode(); // also send new status re XON, XOFF
     keyerState = keyer.sendCode( x ); // send code or do nothing if got zero
  }
  protocol.sendStatus(keyerState);
}

// for debugging; remove in production code
void blik(bool start) {
  if( start ) {
    blikTime = currentTime + 5;
    digitalWrite(LED, HIGH);
  }
  else if( blikTime < currentTime ) {
    digitalWrite( LED, LOW );
  }
}
