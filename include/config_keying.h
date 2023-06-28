#ifndef _CONFIG_KEYING_H_
#define _CONFIG_KEYING_H_

/* This is interface hardware pin definition for keying interface in your hardware using Arduino: 
 * Key line - keying external rig, such as transceiver, CPO
 * PTT line - PTT control of the connected external rig
 * Sidetone - pin generating square wave sidetone. It may follow Key line but it also transmits audible information to operator
 * CPO - similar to Sidetone except it is DC key line unlike Sidetone pin. May be keyed in cases when Key line is not.
 */

#if defined(HW_CHALLENGER_PLAST)

#define CONFIG_KEYING_KEYLINE1 A2
#define CONFIG_KEYING_PTTLINE1 A1
#define CONFIG_KEYING_SIDETONE A5
#define CONFIG_KEYING_CPO      0

#define CONFIG_SIDETONE_MIN_FREQ 300
#define CONFIG_SIDETONE_MAX_FREQ 3000

#elif defined(HW_CHALLENGER2)

#define CONFIG_KEYING_KEYLINE1 D8
#define CONFIG_KEYING_PTTLINE1 D7
#define CONFIG_KEYING_SIDETONE D5
#define CONFIG_KEYING_CPO 0

#else

#error You have no HW configuration for KEY and PTT output

#endif

#define CONFIG_SIDETONE_MIN_FREQ 300
#define CONFIG_SIDETONE_MAX_FREQ 4000
#endif