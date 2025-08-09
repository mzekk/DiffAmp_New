/***************************************************************************
Title: PCA9554
by: AD0ND

This file is free software; you can redistribute it and/or modify
it under the terms of either the GNU General Public License version 2
or the GNU Lesser General Public License version 2.1, both as
published by the Free Software Foundation.
***************************************************************************/

#ifndef PCA9554_h
#define PCA9554_h

#include <Arduino.h>
#include <Wire.h>

//PCA9554 Command Byte
#define INPUTPORT	0x00
#define OUTPUTPORT	0x01
#define POLINVPORT	0x02
#define CONFIGPORT	0x03

// PORT EXPANDER IOs
#define ALLOUTPUT	0x00
#define ALLINPUT	0xFF
#define LED_POL_P 0
#define LED_POL_N 1
#define LED_DIV_25 2
#define LED_DIV_250 3
#define CHARGER_OFF 4
#define RL_COM 5
#define RL1 6
#define RL2 7
// COMMANDS FOR THE PORT EXPANDER
#define ALL_RELAYS_OFF 0
#define ALL_RELAYS_ON 1
#define SET_INPUT_RANGE_250V 2
#define SET_INPUT_RANGE_25V 3
#define SELECT_SMU 4
#define SELECT_DIFF_MEAS 5
#define DISP_PROBE_POLARITY_POS 6
#define DISP_PROBE_POLARITY_NEG 7
#define DISP_INPUT_DIV25 8
#define DISP_INPUT_DIV250 9
#define DISP_OFF 10
#define ALL_LEDS_OFF 11

class PCA9554
{
	public :
		PCA9554(byte SlaveAddress);

		bool twiRead(byte &registerAddress);
		bool twiWrite(byte registerAddress, byte dataWrite);

		bool pinMode(byte pinNumber, bool state);
		bool portMode(byte value);
		
		bool digitalWrite(byte pinNumber, bool state);
		bool digitalWritePort(byte value);
				
		bool digitalRead(byte &pinNumber);
		bool digitalReadPort(byte &value);
		
		bool SetRelay(uint8_t relayCommand);
		bool SetLeds(uint8_t ledCommand);
		
	private :
		int _SlaveAddress;
};

#endif
