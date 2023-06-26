#include "keying.h"
#include "paddle.h"
#include "speed_control.h"
#include "morse.h"

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
unsigned long lastTime;

void setup() {
  currentTime = millis();
  pinMode( LED, OUTPUT );
  keyer.init();
  keyer.setMode(IAMBIC_B);
  keyer.setTimingParameters(speedPaddles, 300, 50);
  paddle.init();
  speedControl->init();
  keyer.enableTone(ENABLED);
  // debugging
  keyer.setTone(300);
  digitalWrite( LED, HIGH );
  delay(133);
  keyer.setTone(1300);
  delay(133);
  digitalWrite( LED, LOW );
  keyer.setTone(0);
  // Serial.begin(57600);
  // lastKs.x = 0xFFFF ;
  // lastPs = 7 ;
  // Serial.println(millis());
  lastTime = millis();
  // lastTs = lastTime - 1; 
}

char message[100];

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
  // keyer
  byte ps = paddle.check();
  KeyingStatus ks = keyer.service( ps ) ; // check and update timing and status
  
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

/* // debug
void reportKeyer(KeyingStatus s ) {
  sprintf( message, " KS %s %s %s", s.busy ? "BUSY" : "IDLE", s.current == 1 ? "DIT" : s.current ? "DAH" : "N/A", s.key ? "ON" : "OFF");
  Serial.println(message);
  sprintf( message, " ON: %lu, OFF: %lu", keyer.getOnTime(), keyer.getOffTime());
  Serial.println( message );
}

void reportPaddle(byte x) {
  sprintf(message, "PADDLE %s", (x == 0? "FREE" :(x == 3 ? "SQUEEZE": (x==2 ? "DAH" : "DIT"))));
  Serial.println(message);
} */