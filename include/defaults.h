/**
 *  Default values to be set at power-up
 *  Structure can be expanded at later stages
 */

#if !defined( DEFAULTS_H )
#define DEFAULTS_H 

#if !defined( word ) || !defined( byte )
#include <Arduino.h>
#endif

struct DefaultValues {
  byte version ; 
  byte wpmPaddle ;
  word weighting ;
  word ditDahFactor ;
  word pttLead ;
  word pttTail ;
  word firstExtension ;
  word sidetonePaddleHz ;

}


#endif