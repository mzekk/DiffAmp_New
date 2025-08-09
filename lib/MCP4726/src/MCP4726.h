/***************************************************************************
Title: MCP4726_h
by: MZ

This file is free software; you can redistribute it and/or modify
it under the terms of either the GNU General Public License version 2
or the GNU Lesser General Public License version 2.1, both as
published by the Free Software Foundation.
***************************************************************************/

//**** MCP4726 1-Chanell 12-BIT DAC ****



#ifndef MCP4726_h
#define MCP4726_h

#include <Arduino.h>
#include <Wire.h>

#define INT_VREF 				0x10 // Unbuffered reference from VDD (supply pin)
#define INT_VREFB 				0x18 // External reference voltage, Unbuffered
#define VDD_VREF				0 	// External reference voltage, Buffered
#define INT_VREF_VAL 			2.048 // Internal reference voltage
#define DAC_GAIN_1X 			0	// Gain of 1
#define DAC_GAIN_2X 			1	// Gain of 2
#define PD0						0	// Not Powered Down (Normal Operation)
#define PD1						1	// Powered Down – VOUT is loaded with 1 kΩ resistor to ground.
#define PD2						2	// Powered Down – VOUT is loaded with 100 kΩ resistor to ground.
#define PD3						3	// Powered Down – VOUT is loaded with 500 kΩ resistor to ground.

//MCP4728 Update Modes
#define FAST_DAC_UPDATE         0x00 // 
#define DAC_UPDATE          	0x10 //
#define DAC_EEP_UPDATE   		0x18 // 
#define SET_OPTIONS				0x80 //

#define R5 						7680 // In ohms, Bottom resistor setting the offset correction circuit gain					
#define R10 					49900// In ohms, Top resistor setting the offset correction circuit gain
#define R22 					12.1 // In ohms, Vout amplifier positive input offset correction bottom resistor
#define R17 					3920 // In ohms, Vout amplifier positive input offset corection top resistor
		


#define nUDAC 1	// DAC Outputs updated by LDAC pin

class MCP4726
{
	public :
		MCP4726(byte SlaveAddress, byte VrefMode, byte Gain);
		bool writeDAC(uint16_t DAC_value, bool eepUpdate);
		bool writeDACFast(uint16_t DAC_values);
		bool setOptions(uint8_t options);
		uint16_t float2dac(float voltage);
		uint16_t voffs2DAC(float voffs); 

		
	private :
		int _SlaveAddress;
		int _VrefMode;
		int _Gain;
};

#endif
