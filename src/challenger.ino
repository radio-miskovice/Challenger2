#include "keying.h"
#include "paddle.h"
#include "speed_control.h"
#include "protocol.h"
#include "morse.h"

// debugging
unsigned long blikTime = 0 ;
const byte LED = CONFIG_CMD_MODE_LED ;
void blik(bool);

/* GLOBAL VARIABLES */
KeyingSource keySource = SRC_PADDLE ;
int speedPaddles = 25 ;
int speedBuffer = 20 ;
int speedCommand = 20 ;
unsigned long currentTime ;

void setup() {
  // BUFFER indicator setup
  pinMode( LED_BUILTIN, OUTPUT );
  digitalWrite( LED_BUILTIN, HIGH );
  // basic component setup
  protocol.init();
  keyer.init();
  keyer.setDefaults();
  paddle.init();
  speedControl->init(); // includes also command mode LED
  speedControl->setMinMax(15,46);
  speedControl->setValue(speedPaddles);
   // initial beep and flash
  keyer.setTone(300);
  digitalWrite( LED, HIGH );
  delay(133);
  keyer.setTone(1300);
  delay(133);
  digitalWrite( LED, LOW );
  keyer.setTone(0);
  delay(100);
  // initial
  currentTime = millis();
  keyer.service(0);
  digitalWrite( LED_BUILTIN, LOW );
  // testing parameters
  protocol.enablePaddleEcho( ON );
}

char message[100];
int index = 0 ;
KeyerState kkk ;

char* binary( word x, byte limit = 16 ) {
  word testvalue = 1 << (limit - 1);
  for( byte i = 0; i<limit; i++ ) {
    message[i] = (x & testvalue) ? '1' : '0' ;
    x = x << 1 ;
  }
  message[limit] = 0;
  return message ;
}

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
  byte paddleState = paddle.check();
  KeyerState keyerState = keyer.service( paddleState ) ; // check and update timing and status
  protocol.service(keyerState); // check incoming data and execute command if necessary
  // if( keyerState.breakIn == ON ) {
  //   protocol.stopBuffer(); // will also send WK status "breakin" and "ready"
  // }
  if( keyer.canAccept() ) { // can send from buffer?
     byte x = protocol.getNextMorseCode(); // also send new status re XON, XOFF
     keyerState = keyer.sendCode( x ); // send code or do nothing if got zero
  }
  if( keyerState.source == SRC_PADDLE && keyerState.busy == READY ) {
    word code = keyer.getCollectedCode();
    byte ascii = morse.decodeMorse(code);
    if( ascii >= ' ' ) protocol.sendPaddleEcho(ascii);

    // if( code > 0 ) {
    //   byte ascii = morse.lookupCode(code);
    // }
  }
  protocol.sendStatus(keyerState);
}

// speed change indicator
void blik(bool start) {
  if( start ) {
    blikTime = currentTime + 5;
    digitalWrite(LED, HIGH);
  }
  else if( blikTime < currentTime ) {
    digitalWrite( LED, LOW );
  }
}