# Keying Mode

## General rules

- paddle status is checked when the current state is IDLE (may be checked before but is read right at that moment)

## Iambic A

- when left paddle is pressed, send DIT
- when right paddle is pressed, send DAH
- when squeeze, play the opposite of previous
- when nothing, do nothing

## IAMBIC_B

- when left paddle is pressed, send DIT
- when right paddle is pressed, send DAH
- when squeeze, choose the opposite
- when nothing:
  - if previous was DIT, play one more DIT
  - if previous was DAH, play one more DAH
  - if previous was squeeze, play opposite of previous
