#ifndef _CONFIG_KEYING_
#define _CONFIG_KEYING_

/* This is interface hardware pin definition for keying interface in your hardware using Arduino: 
 * Key line - keying external rig, such as transceiver, CPO
 * PTT line - PTT control of the connected external rig
 * Sidetone - pin generating square wave sidetone. It may follow Key line but it also transmits audible information to operator
 * CPO - similar to Sidetone except it is DC key line unlike Sidetone pin. May be keyed in cases when Key line is not.
 */

#define CONFIG_KEYING_KEYLINE1 LED_BUILTIN
#define CONFIG_KEYING_PTTLINE1 7
#define CONFIG_KEYING_SIDETONE 5
#define CONFIG_KEYING_CPO      0

#define CONFIG_SIDETONE_MIN_FREQ 300
#define CONFIG_SIDETONE_MAX_FREQ 3000

#endif