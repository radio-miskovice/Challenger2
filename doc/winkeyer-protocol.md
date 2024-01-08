# Winkeyer Protocol Implementation in Challenger 2

## Admin Commands


| code hex | code dec | name         | Remarks                                                |
| :------: | :------: | ------------ | ------------------------------------------------------ |
| 0x01     |        1 | REBOOT       | Reboots CPU                                            |
| 0x02     |        2 | OPEN HOST    | does not change behavior, returns revision number (22) |
| 0x03     |        3 | CLOSE HOST   | does nothing                                           |
| 0x04     |        4 | ECHO         | returns parameter                                      |
| 0x05     |        5 | PADDLE_A2D   | returns parameter, no action                           |
| 0x06     |        6 | SPEED_A2D    | returns parameter, no action                           |
| 0x07     |        7 | ignore       | no action                                              |
| 0x08     |        8 | ignore       | no action                                              |
| 0x0A     |        9 | SET MODE WK1 | no action (ignore)                                     |
| 0x0B     |       10 | SET MODE WK2 | no action (ignore)                                     |

Any other admin command is ignored.

## Regular commands

| code hex | code dec | name         | Remarks                                                |
| :------: | :------: | ------------ | ------------------------------------------------------ |
| 0x01 | 1 | Sidetone for paddle only |  |
| 0x02 | 2 | Set WPM |  |
| 0x03 | 3 | Set Weighting |  |
| 0x04 | 4 | Set PTT head, tail |  |
| 0x05 | 5 | Set WPM speed range | |
| 0x06 | 6 | ignore | no action |
| 0x07 | 7 | Get WPM speed | returns speed in WPM in WK2 format |
| 0x0A | 10 | clear buffer |  |
| 0x0B | 11 | Key ON | key down for 15 seconds |
| 0x0D | 13 | Set effective (Farnsworth) WPM |  |
| 0x0E | 14 | Set mode parameters |  |
| 0x0F | 15 | Set defaults |  |
| 0x10 | 16 | Set 1st extension |  |
| 0x11 | 16 | Set QSK compensation |  |
| 0x15 | 21 | Get Winkeyer status |  |
| 0x16 | 22 | Set dah:dit ratio |  |

Any other commands not listed above are silently ignored.