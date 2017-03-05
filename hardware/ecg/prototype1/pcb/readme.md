# ECG PCB

![schematic](./ecg_sch.png)

![pcb layout](./ecg_brd.png)

There were some bugs on the board:

1. ADS1929R missing crucial feedback from RLDOUT via 1Mohm resistor with 1.5nF capacitor in parallel with resistor.
Ref datasheet figure 52. Not clear if this bug could have been worked around in software.

2. CLKSEL needs to be tied high to enable internal clock.

3. Coin cell solder mask wrong.
