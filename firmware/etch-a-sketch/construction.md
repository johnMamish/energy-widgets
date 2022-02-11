Left-hand side motor harvester
  - sense pin 1: gnd
  - sense pin 2: P3.0
  - sense pin 3: P3.1
  - sense pin 4: P1.3

Right-hand side motor harvester
  - sense pin 1: gnd
  - sense pin 2: P3.2
  - sense pin 3: P3.3
  - sense pin 4: P1.4

Screen
  - EIN: NC
         When EMD is high; this pin toggles VCOM inside the LCD.
         When EMD is low, it should be tied to VSS. Adafruit's carrier board has a pulldown
         resistor on EIN, so we can leave it NC.
  - DISP: msp430 P1.2
          Toggles whether to show something on the display or not.
  - EMD: NC
         When EMD is high, an external VCOM signal needs to be supplied on EIN.
         Adafruit's board ties EMD low with a pulldown, so we can leave it disconnected
         and supply a VCOM signal over SPI.
  - CS:  msp430 P8.3 GPIO
  - DI:  msp430 P5.0 UCB1SIMO
  - CLK: msp430 P5.2 UCB1CLK
  - GND: msp430 GND
  - 3V3: msp430 "3v3"
  - VIN: NC
