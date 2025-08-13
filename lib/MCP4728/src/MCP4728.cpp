#include <MCP4728.h>

MCP4728::MCP4728(byte SlaveAddress, byte Vref)
{
	_SlaveAddress = SlaveAddress;
	_Vref = Vref;
}


bool MCP4728::writeDAC(uint16_t value, uint8_t channel, bool eepUpdate)
{
	uint8_t cmd = SEQ_DAC_UPDATE ;
	Wire.beginTransmission(_SlaveAddress); 				// start transmission to device 
	if (eepUpdate)
		cmd = SINGLE_DAC_EEP_UPDATE;
	Wire.write(cmd + (channel<<1) + nUDAC); 			// sends DAC register address to write
	Wire.write(value/0x100 & 0x0F + (_Vref << 7));      // sends upper value (4 msb bita), VREF bit7 and PD0/PD1 set to 0
	Wire.write(value & 0xFF);              				// sends lower value byte 
  	Wire.endTransmission();           					// stop transmitting
	if (Wire.endTransmission(false) == 0) 
		return true; // end transmission
	return false;
}

bool MCP4728::writeAllDACs(uint16_t (&DAC_values)[4], bool eepUpdate)
{
	uint8_t i, val;
	Wire.beginTransmission(_SlaveAddress); 				// start transmission to device 
	if (eepUpdate)
		Wire.write(SEQ_DAC_EEP_UPDATE + nUDAC); 		// sends DAC register address to write	
	for (i = 0; i < 4; i++)	{
		if (!eepUpdate)
			Wire.write(SEQ_DAC_UPDATE + (i<<1) + nUDAC); 	// sends DAC register address to write	
		val = _Vref << 7;
		val += (DAC_values[i]/0x100) & 0x0F;
		Wire.write(val);            	// sends upper value (4 msb bita), VREF bit7 and PD0/PD1 set to 0
		Wire.write(DAC_values[i] & 0xFF);              				// sends lower value byte 
	}
  	Wire.endTransmission();           					// stop transmitting
	if (Wire.endTransmission(false) == 0) 
		return true; // end transmission
	return false;
}

bool MCP4728::writeAllDACsFast(uint16_t (&DAC_values)[4])
{
	uint8_t i;
	Wire.beginTransmission(_SlaveAddress); 				// start transmission to device 
	for (i = 0; i < 4; i++)	{
		Wire.write(DAC_values[i]/0x100 & 0x0F);            	// sends upper value (4 msb bita), VREF bit7 and PD0/PD1 set to 0
		Wire.write(DAC_values[i] & 0xFF);              				// sends lower value byte 
	}
  	Wire.endTransmission();           					// stop transmitting
	if (Wire.endTransmission(false) == 0) 
		return true; // end transmission
	return false;
}

void MCP4728::updateDACs(void)
{
	digitalWrite(LDAC_CHRG, LOW);
	pinMode(LDAC_CHRG, OUTPUT);		// Set LDAC_CHRG low to control the DAC data loading
	delayMicroseconds(100);
	pinMode(LDAC_CHRG, OUTPUT);		// Release LDAC_CHRG pin as input to detect charger presence
	digitalWrite(LDAC_CHRG, LOW);	// Not needed, but just in case
}

bool MCP4728::setVref(uint8_t vrefConf){
	Wire.beginTransmission(_SlaveAddress);
	uint8_t cmd = SET_DAC_VREF + vrefConf;
	Wire.write(cmd);
	Wire.endTransmission();
	if (Wire.endTransmission(false) == 0) 
		return true; // end transmission
	return false;
}

bool MCP4728::setGain(uint8_t gainConf){
	Wire.beginTransmission(_SlaveAddress);
	uint8_t cmd = SET_DAC_GAIN + gainConf;
	Wire.write(cmd);
	Wire.endTransmission();
	if (Wire.endTransmission(false) == 0) 
		return true; // end transmission
	return false;
}

uint16_t MCP4728::float2dac(float voltIn) 
{
	uint16_t dacVal;
	dacVal = int(voltIn * 4095.0 / INT_VREF_VAL_DAC4);
	return dacVal;
}

float MCP4728::vampp2vdac(float vpos) 
{
	float vdac;
	vdac = K1_VAMPP - K2_VAMPP * vpos;
	return vdac;
}

float MCP4728::vampn2vdac(float vneg) 
{
	float vdac;
	vdac = K1_VAMPN - K2_VAMPN * vneg;
	return vdac;
}