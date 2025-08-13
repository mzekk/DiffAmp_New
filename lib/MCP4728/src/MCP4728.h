/***************************************************************************
Title: MCP4728_h
by: MZ

This file is free software; you can redistribute it and/or modify
it under the terms of either the GNU General Public License version 2
or the GNU Lesser General Public License version 2.1, both as
published by the Free Software Foundation.
***************************************************************************/

//**** MCP4728 4-Chanell 12-BIT DAC ****

#ifndef MCP4728_h
#define MCP4728_h

#include <Arduino.h>
#include <Wire.h>
#include <pin_config.h>

//#define LDAC_CHRG                    12 // This is a double function IO

#define VREF_CONF 1					// 1 = Internal 2.048V reference, 0 = External VDD (supply pin) reference
#define INT_VREF_VAL_DAC4 	2.048 				// Internal reference voltage
#define INT_VREF_ALL 		0x0F			// All DAC channels use the same internal reference voltage
#define GAIN_ALL 			0x00				// All DAC channels use the same gain = 0


//MCP4728 Update Modes
#define FAST_DAC_UPDATE         0x00 // 
#define SEQ_DAC_UPDATE          0x40 //
#define SEQ_DAC_EEP_UPDATE     	0x50 //
#define SINGLE_DAC_EEP_UPDATE   0x58 // 
#define SET_DAC_VREF            0x80 //
#define SET_DAC_GAIN            0xC0 //

#define nUDAC 1			// DAC Outputs updated by LDAC pin
#define PDMODE 0x60		// DAC Channels Power Down Mode


// -V2P5 Regulator Characteristics
#define RTOP_V2P5P 	39200.0f
#define RBOT_V2P5P 	34000.0f
#define VFB_V2P5P 	-1.2f
#define V2P5_NEG 	VFB_V2P5P*(RTOP_V2P5P + RBOT_V2P5P)/RBOT_V2P5P
// +VAMP Regulator Characteristics
#define RTOP_VAMPP 	100000.0f
#define RBOT_VAMPP 	19600.0f
#define RDAC_VAMPP 	15800.0f
#define VFB_VAMPP 	1.235f
// VDAC = K1_VAMPN - K2_VAMPN*VAMP 
#define K1_VAMPP	VFB_VAMPP*(1 + RDAC_VAMPP/RTOP_VAMPP + RDAC_VAMPP/RBOT_VAMPP)
#define K2_VAMPP	RDAC_VAMPP/RTOP_VAMPP
// -VAMP Regulator Characteristics
#define RTOP_VAMPN 	100000.0f
#define RBOT_VAMPN 	21000.0f
#define RDAC_VAMPN 	15800.0f
#define VFB_VAMPN 	-1.2f
#define VREF_VAMPN	V2P5_NEG
// VDAC = K1_VAMPN - K2_VAMPN*VAMP 
#define K1_VAMPN	VFB_VAMPN + VFB_VAMPN*RDAC_VAMPN/RTOP_VAMPN + RDAC_VAMPN*(VFB_VAMPN-VREF_VAMPN)/RBOT_VAMPN
#define K2_VAMPN	RDAC_VAMPN/RTOP_VAMPN

class MCP4728
{
	public :
		MCP4728(byte SlaveAddress, byte Vref);
		bool writeDAC(uint16_t DAC_value, uint8_t channel, bool eepUpdate);
		bool writeAllDACs(uint16_t (&DAC_values)[4], bool eepUpdate);
		bool writeAllDACsFast(uint16_t (&DAC_values)[4]);
		void updateDACs(void);
		bool setVref(uint8_t vrefConf);
		bool setGain(uint8_t gainConf);
		uint16_t float2dac(float voltage);
		float vampp2vdac(float vpos);
		float vampn2vdac(float vneg); 

		
	private :
		int _SlaveAddress;
		int _Vref;
};

#endif
