/***************************************************************************
Title: MAX17040_h
by: MZ

This file is free software; you can redistribute it and/or modify
it under the terms of either the GNU General Public License version 2
or the GNU Lesser General Public License version 2.1, both as
published by the Free Software Foundation.
***************************************************************************/

//**** MAX17040G - 1-CELL BATTERY FUEL-GAUGE****

#ifndef MAX17040_h
#define MAX17040_h

#include <Arduino.h>
#include <Wire.h>

//MAX17040 Command Byte
#define MAX17040G_VCELL_HIGH    0x02 // Battery voltage (Bit 11:4) - 1.25mV per bit
#define MAX17040G_VCELL_LOW     0x03 // Battery voltage (Bit 3:0)  - 1.25mV per bit
#define MAX17040G_SOC_MSB       0x04 // Battery State of Charge MSB - 1bit = 1%   
#define MAX17040G_SOC_LSB       0x05 // Battery State of Charge MSB - 1bit = (1/128)%   
#define MAX17040G_MODE_MSB      0x06 // Mode command, only QUICK-START (0x4000) is allowed parameter    
#define MAX17040G_MODE_LSB      0x07 // Mode command, only QUICK-START (0x4000) is allowed parameter   
#define MAX17040G_VER_MSB       0x08 // IC Version MSB 
#define MAX17040G_VER_LSB       0x09 // IC Version MSB 
#define MAX17040G_COMP_MSB      0x0C // Battery algorithm compensation, default is 0x9700 
#define MAX17040G_COMP_LSB      0x0D // Battery algorithm compensation, default is 0x9700  
#define MAX17040G_CMD_MSB       0xFE // Special Command MSB - Only 0x0054 is allowed
#define MAX17040G_CMD_LSB       0xFF // Special Command LSB - Only 0x0054 is allowed


class MAX17040
{
	public :
		MAX17040(byte SlaveAddress);
		bool getBatterySoC(float &batterySoC);
		bool getBatteryVoltage(float &batteryVoltage);
		
	private :
		int _SlaveAddress;
};

#endif
