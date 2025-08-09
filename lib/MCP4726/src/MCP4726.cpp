#include <MCP4726.h>

MCP4726::MCP4726(byte SlaveAddress, byte VrefMode, byte Gain)
{
	_SlaveAddress = SlaveAddress;
	_VrefMode = VrefMode;
	_Gain = Gain;
}


/**
 * Writes a value to the DAC register and updates the EEPROM if specified.
 *
 * @param value the value to be written to the DAC register
 * @param eepUpdate indicates whether to update the EEPROM
 *
 * @return true if the transmission was successful, false otherwise
 *
 * @throws None
 */
bool MCP4726::writeDAC(uint16_t value, bool eepUpdate)
{
	uint8_t cmd = DAC_UPDATE ;
	Wire.beginTransmission(_SlaveAddress); 				// start transmission to device 
	if (eepUpdate)
		cmd = DAC_EEP_UPDATE;
	Wire.write(cmd + (_VrefMode << 3) + _Gain); 	// sends DAC register address to write
	Wire.write(value/0x100 & 0x0F);            	// sends upper value (4 msb bita), VREF bit7 and PD0/PD1 set to 0
	Wire.write(value & 0xFF);              				// sends lower value byte 
  	Wire.endTransmission();           					// stop transmitting
	if (Wire.endTransmission(false) == 0) 
		return true; // end transmission
	return false;
}


bool MCP4726::writeDACFast(uint16_t DAC_value)
{
	uint8_t i;
	Wire.beginTransmission(_SlaveAddress); 				// start transmission to device 
	Wire.write(DAC_value/0x100 & 0x0F);            	// sends upper value (4 msb bita), VREF bit7 and PD0/PD1 set to 0
	Wire.write(DAC_value & 0xFF);              				// sends lower value byte 
  	Wire.endTransmission();           					// stop transmitting
	if (Wire.endTransmission(false) == 0) 
		return true; // end transmission
	return false;
}

bool MCP4726::setOptions(uint8_t options){
	Wire.beginTransmission(_SlaveAddress);
	uint8_t cmd = SET_OPTIONS + options;
	Wire.write(cmd);
	Wire.endTransmission();
	if (Wire.endTransmission(false) == 0) 
		return true; // end transmission
	return false;
}


uint16_t MCP4726::float2dac(float voltIn) 
{
	uint16_t dacVal;
	dacVal = int(voltIn * 4095.0 / INT_VREF_VAL);
	return(dacVal & 0xFFF);
}

uint16_t MCP4726::voffs2DAC(float voffs) 
{
	uint16_t dacVal;
	dacVal = int(4095.0 * voffs * (R17 + R22)/ (INT_VREF_VAL*R22*(1+R10/R5)));
	return dacVal & 0xFFF;
}