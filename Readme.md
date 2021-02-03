# Radio Interface Firmware #

This Repository is for the microcontroller utilized in the PSCR MCV QoE measurement system. This code is utilized by the measurement systems described in [NIST IR 8206](https://doi.org/10.6028/NIST.IR.8206) and [NIST IR 8275](https://doi.org/10.6028/NIST.IR.8275). The code is written for the TI MSP430F5529 processor and was developed using a MSP-EXP430F5529LP LaunchPad board.

The following projects require the use of this firmware:
- https://github.com/usnistgov/mouth2ear
- https://github.com/usnistgov/accessTime

The code was compiled and loaded using TI Code Composer Studio (CCS) IDE, available here: http://www.ti.com/tool/CCSTUDIO.

## Matlab Test ##

The Matlab-test folder contains code to test the microcontroller. This code only requires base MATLAB and allows the functionality of the microcontroller board to be tested. This can be used with a newly programed board as a quick check that everything is working as expected. The test can be started as follows.
```
RItest
```

To complete all parts of the test you will need a multimeter and/or an oscilloscope and a wire to connect P6.0 to 3.3V. Alternately a variable bench supply can be used to determine the switching threshold. If all that is desired is to determine that the firmware was loaded correctly and able to communicate with MATLAB, simply vusually checking that the LEDs come on is sufficient.

## Commands ##
The command line interface implemented by the software is primarily intended to be used with the radioInterface MATLAB class. It can, however, be easily used manually with a serial terminal program. Commands are case sensitive. The commands are listed below:

- **help**: If help is called with no arguments, it displays a list of all possible commands. If the name of a command is passed to help, then it gives a short description of what the command does and what arguments to pass to it.
- **ptt**: The ptt command activates or deactivates the push to talk signal. If no arguments are given, it will display the ptt status. If the first argument is on or off, then the ptt signal is activated or deactivated respectively. If the first argument is delay then the second argument is the delay in seconds and the ptt signal will be activated after that number of seconds. The number of seconds is adjustable with about 1 ms resolution and can be up to 64 seconds. The actual delay is printed by the ptt command when the delay argument is given.
- **devtype**: The devtype command is for identification of the device. It is mainly so the MATLAB class can search for a usable device on one of the available serial ports.
- **LED**: The LED command will turn on or off one of the LEDs on the board.
- **closeout**: The closeout command is primarily meant to be called when the radioInterface class is deleted. It deactivates the ptt signal and turns off all of the LEDs. It takes no arguments.
- **temp**: The temp command is used to measure both the MSP430 temperature and an optional external thermistor. The thermistor connects to analog channel 5 on P6.5. The code was used with a Cantherm MF52A2103J3470 thermistor as the bottom leg (connected to ground) of a voltage divider with the other resistor, connected to 3.3V, being a 10 k ohm. The internal temperature measures the die temperature of the MSP430 which is generally a bit higher than ambient due to the internal voltage regulator.
- **id**: The id command is used to get a unique ID for each board. The die record is used to return a hex string that is unique to each IC.
- **bsl**: The bsl command puts the processor in bootstrap loader mode. In bsl mode the virtual COM port will no longer be available and a USB HID device for the bsl will instead be used. The only way to return to the command line interface is to reset the processor. This intended for use with a possible future firmware upgrade tool.

## Supplies List ##

- MSP430F5529LP
- Perfboard
- 2x10 0.1 in. pitch female headers
- Connector to audio output of audio interface
- Connector to audio inputof audio interface
- Resistors: 330 Ω, 2.2 kΩ
- Diode
- Connector for radio
- 4N35 optocoupler

## Schematic ##

Below is the schematic for the microcontroller daughter board. The pieces are described in mor detail in section 4.3 of the [access time paper](https://doi.org/10.6028/NIST.IR.8275).

![The schematic for the microcontroller contains two sets dual row 20 pin headers. The input audio connects to the P6.0 on the left header through a diode with the cathode connected to P6.0. There is a 2.2 kohm resistor from P6.0 to ground. The start signal connects to P2.5 of the right header. There is a 330 ohm resistor bwtween P8.2 of the right header and the anode of the LED of a 4N35 opto-isolator. The cathode of the 4N35 LED is grounded. The output transistor collector and emiter pins are connected to the radio PTT. The left side of the left header, from top to bottom, has the following connections: +3.3 V, P6.5, P3.4, P3.3, P1.6, P6.6, P3.2, P2.7, P4.2, and P1.4. The right side of this header, from top to bottom, has the following connections: +5 V, ground, P6.0, P6.1, P6.2, P6.3, P6.4, P7.0, P3.6, and P3.5. The left side of the right header, from top to bottom, has the following connections: P2.5, P2.4, P1.5, P1.4, P1.3, P1.2, P4.3, P4.0, P3.7, and P8.2. The right side of the right header, from top to bottom, has the following connections : ground, P2.0, P2.2, P7.4, reset, P3.0, P3.1, P2.6, P2.3, and P8.1. ](schematic.svg)
