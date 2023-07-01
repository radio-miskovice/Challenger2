# Send Status Events in WinKeyer

 - response to command 0x15 (in protocol.executeCommand() )
 - change of speed => status 0x80 + wpm offset from minimum (in the main event loop after change of speed is detected)
 - breakin condition => send 0xC6, immediately followed by 0xC0 (in the main event loop when break-in event is detected)
 - start sending buffer: 0xC4 (in protocol.service() when the first character is put in buffer)
 - buffer full: 0xC5 (BUSY + XOFF)
 - buffer can again accept characters: 0xC4 (hysteresis between XOFF and !XOFF should be >= 10 chars?)

