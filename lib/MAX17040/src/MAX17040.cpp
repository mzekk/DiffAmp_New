#include <MAX17040.h>

MAX17040::MAX17040(byte SlaveAddress)
{
	_SlaveAddress = SlaveAddress;
}


bool MAX17040::getBatterySoC(float &batterySoc)
{
	uint16_t cv;
	Wire.beginTransmission(_SlaveAddress); // start transmission to device 
	Wire.write(MAX17040G_SOC_MSB); // sends register address to read from
	Wire.endTransmission(false); // end transmission
	
	if (Wire.requestFrom(_SlaveAddress, 2)) // send data n-bytes read
	{
		batterySoc = Wire.read(); // receive DATA
		batterySoc += float(Wire.read())/256;
		if(batterySoc > 100)
			batterySoc = 100;
		return true;
	}
	return false;
}

bool MAX17040::getBatteryVoltage(float &batteryVoltage)
{
	Wire.beginTransmission(_SlaveAddress); // start transmission to device 
	Wire.write(MAX17040G_VCELL_HIGH); // sends register address to read from
	Wire.endTransmission(false); // end transmission
	
	if (Wire.requestFrom(_SlaveAddress, 2)) // send data n-bytes read
	{
		batteryVoltage = Wire.read() * 0x100; // receive DATA
		batteryVoltage += Wire.read();
		batteryVoltage *= 0.000078125;	// 1.25mV/bit divided by 16 since bit 0 to 3 are not used and set to 0
		return true;
	}
	return false;
}