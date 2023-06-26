/**
 * Morse code converter and keying processor
 * Loosely based on code developed by Goody, K3NG and Petr, OK1FIG
 * [CC BY-NC-4.0] Creative commons Licence 4.0
 * https://creativecommons.org/licenses/by-nc/4.0/
 * Jindrich Vavruska, jindrich@vavruska.cz
 **/

#include "morse.h"

MorseEngine morse = MorseEngine();

/** MORSE CODE CONVERSION TABLE
 * How to read code:
 * Each morse code character is binary encoded. It is interpreted from MSB to LSB
 * (MSB - first dit or dash) 0 = dit, 1 = dash.
 *
 * The least significant bit 1 in the code is end marker ("stop bit") => 0x80 means send end-of-character space (CHARSPACE).
 * 0x00 represents NULL, or invalid morse code, always to be interpreted as "nothing to send".
 *
 * Code is interpreted by reading MSB, then shift left. Bit value 1 means DAH, bit value 0 means DIT.
 * Width is limited by 8 bits. Maximum possible code length is hence 7 elements.
 *
 * This code table is based on recommendation ITU-R M.1677-1
 * Morse code for exclamation (!) is adopted from https://morsecode.world/international/morse2.html
 * Dollar sign ($), semicolon (;), underscore (_) are copied form somewhere, probably K3NG source code
 * Signals/prosigns: + = AR, & = AS, * = BK, '(' = KN, > = SK should be compatible with K3NG and WinKeyer protocol
 *
 */
const byte CODE[] = {
    MORSE_SPACE, // space; will send +4T pause, together with 3T charspace = 7T
    0b10101110,  // ! unofficial
    0b01001010,  // " RR
    0,           // #
    0b00010011,  // $ VU, unofficial
    0,           // %
    0b01000100,  // & AS
    0b01111010,  // ' JN
    0b10110100,  // ( KN
    0b10110110,  // ) KK
    0b10001011,  // * BK
    0b01010100,  // + AR
    0b11001110,  // , 
    0b10000110,  // -
    0b01010110,  // .
    0b10010100,  // stroke /    //--- numbers --//
    0b11111100,  // 0
    0b01111100,  // 1
    0b00111100,  // 2
    0b00011100,  // 3
    0b00001100,  // 4
    0b00000100,  // 5
    0b10000100,  // 6
    0b11000100,  // 7
    0b11100100,  // 8
    0b11110100,  // 9    //--- punctuation ---//
    0b11100010,  // : OS
    0b10101010,  // ; NC unofficial
    0b01010100,  // < = + = AR
    0b10001100,  // = BT
    0b00010110,  // > SK
    0b00110010,  // ?
    0b01101010,  // @ AC
    0b01100000,  // A
    0b10001000,  // B
    0b10101000,  // C
    0b10010000,  // D
    0b01000000,  // E
    0b00101000,  // F
    0b11010000,  // G
    0b00001000,  // H
    0b00100000,  // I
    0b01111000,  // J
    0b10110000,  // K
    0b01001000,  // L
    0b11100000,  // M
    0b10100000,  // N
    0b11110000,  // O
    0b01101000,  // P
    0b11011000,  // Q
    0b01010000,  // R
    0b00010000,  // S
    0b11000000,  // T
    0b00110000,  // U
    0b00011000,  // V
    0b01110000,  // W
    0b10011000,  // X
    0b10111000,  // Y
    0b11001000,  // Z
    0,           // [
    0,           // backslash
    0,           // ]
    0,           // ^ caret
    0b001101100, // _ underscore UK, unofficial
};
const word CODE_SIZE = sizeof(CODE) / sizeof(CODE[0]);

/**
 * @param ascii ASCII letter to be converted
 * @return zero if ascii is not defined in Morse code, otherwise returns binary morse code
 */
byte MorseEngine::asciiToCode(byte ascii)
{
  if (ascii == '|') return MORSE_CHARSPACE; // half space
  if (ascii < 0x20 || ascii >= 0x7B) return 0; // out of range
  // convert lowercase letter to uppercase
  if (ascii > 0x60) ascii -= 0x20;
  // finally, subtract code table offset
  ascii -= 0x20;
  return (CODE[ascii]);
}

/**
 * Converts selected UTF-8 characters to morse code
 * Valid codes only for German letters ä, ö, ü and Russian letters ш, ч, ю, я
 *
 * @param prefix UTF-8 code page prefix. Only Latin-1 and Cyrillic prefixes are accepted.
 * @param utf8Char the second UTF-8 byte. Only German and Russian letters specified above are accepted.
 * @return unsigned char binary morse code for accepted letters, otherwise returns zero (invalid morse code)
 *
 * Latin-1 letters
 * Ä U+00C4  ä U+00E4  C384, C3A4 : morse code .-.- (binary 0b01011000, hex 0x58)
 * Ö U+00D6  ö U+00F6  C396, C3B6 : morse code ---. (binary 0b11101000, hex 0xE8)
 * Ü U+00DC  ü U+00FC  C39C, C3BC : morse code ..-- (binary 0b00111000, hex 0x38)
 *
 * Cyrillic letters
 * Ч U+0427  ч U+0447  D0A7, D0C7 : morse code ---. (binary 0b11101000, hex 0xE8)
 * Ш U+0428  ш U+0448  D0A8, D0C8 : morse code ---- (binary 0b11111000, hex 0xF8)
 * Ю U+042E  ю U+044E  D0AE, D0CE : morse code ..-- (binary 0b00111000, hex 0x38)
 * Я U+042F  я U+044F  D0AF, D0CF : morse code .-.- (binary 0b01011000, hex 0x58)
 */
byte MorseEngine::utf8ToCode(byte prefix, byte utf8Char)
{
  unsigned int utf8Code = 256 * prefix + utf8Char;
  switch (utf8Code)
  {
  case 0xC384:
  case 0xC3A4:
  case 0xD0AF:
  case 0xD0CF:
    return 0x58;

  case 0xC396:
  case 0xC3B6:
  case 0xD0A7:
  case 0xD0C7:
    return 0xE8;

  case 0xC39C:
  case 0xC3BC:
  case 0xD0AE:
  case 0xD0CE:
    return 0x38;

  case 0xD0A8:
  case 0xD0C8:
    return 0xF8;
  }
  return 0;
}

/**
 * Decodes morse code collected from paddles. Collected code has high stop bit
 * and LSB last morse code element, LSB-aligned. Therefore it must be shifted in order
 * to match morse codes in conversion table, MSB-aligned and the stop bit must be added
 * after the lowest morse code bit.
 * This method is called in holdElementDuration because there's plenty of time to complete.
 **/
char MorseEngine::decodeMorse(word code) {
  code = (code << 8) + 0x80 ; // shift morse code to upper byte and put stop bit
  while( code & 0xFE00 ) { // repeat until MSB shifts to LSB and the stop mark is at 0x0100
    code = code >> 1 ;
  }
  /* phase 2 - morse code lookup in code table */
  if (code == 0x80)
    return (' '); // unlikely to happen;
  word size = sizeof(CODE) / sizeof(CODE[0]);
  // lookup
  for (unsigned char result = 0; result < size; result++)
  {
    if (code == CODE[result]) return (result + 0x20);
  }
  return 0;
}
