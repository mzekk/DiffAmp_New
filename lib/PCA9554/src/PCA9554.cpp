#include <PCA9554.h>

PCA9554::PCA9554(byte SlaveAddress)
{
	_SlaveAddress = SlaveAddress;
}


bool PCA9554::twiRead(byte &registerAddress)
{
	Wire.beginTransmission(_SlaveAddress);
  	Wire.write(registerAddress);

  	if(Wire.endTransmission() == 0)
  	{
    		delay(15);
    		Wire.requestFrom(_SlaveAddress, 1, true);
    		while(Wire.available() < 1);
    		registerAddress = Wire.read();
    		return true;
  	}
  	return false;
}

bool PCA9554::twiWrite(byte registerAddress, byte dataWrite)
{
	Wire.beginTransmission(_SlaveAddress);
  	Wire.write(registerAddress);
  	Wire.write(dataWrite);

  	if(Wire.endTransmission() == 0)
    		return true;
  	return false;
}

bool PCA9554::pinMode(byte pinNumber, bool state)
{
	byte oldValue = CONFIGPORT;
	if(this->twiRead(oldValue) && (pinNumber <= 7))
	{
		if(!state)
		{
			oldValue |= (1 << pinNumber);
			if(this->portMode(oldValue))
				return true;
			return false;
		}
		else if(state)
		{
			oldValue &= ~(1 << pinNumber);
			if(this->portMode(oldValue))
				return true;
			return false;
		}
	}
	return false;
}

bool PCA9554::portMode(byte value)
{
	if(this->twiWrite(CONFIGPORT, value))
		return true;
	return false;
}


bool PCA9554::digitalWrite(byte pinNumber, bool state)
{
	byte oldValue = OUTPUTPORT;
	if(this->twiRead(oldValue) && pinNumber <= 7)
	{
		if(state)
		{
			oldValue |= (1 << pinNumber);
			if(this->digitalWritePort(oldValue))
				return true;
			return false;
		}
		else if(!state)
		{
			oldValue &= ~(1 << pinNumber);
			if(this->digitalWritePort(oldValue))
				return true;
			return false;
		}
	}
	return false;
}



bool PCA9554::digitalWritePort(byte value)
{
	if(this->twiWrite(OUTPUTPORT, value))
		return true;
	return false;
}



bool PCA9554::digitalRead(byte &pinNumber)
{
	byte oldValue = INPUTPORT;
	if(this->twiRead(oldValue) && (pinNumber <= 7))
	{
		oldValue &= (1 << pinNumber);
		if(oldValue > 0) pinNumber = 1;
		else pinNumber = 0;
		return true;
	}
	return false;
}


bool PCA9554::digitalReadPort(byte &value)
{
	value = INPUTPORT;
	if(this->twiRead(value))
		return true;
	return false;
}


bool PCA9554::SetRelay(uint8_t relayCommand)
{
	bool opOK = false, rlatched = true;
	switch(relayCommand){
		case ALL_RELAYS_OFF:
			opOK = this -> digitalWrite(RL_COM, LOW);
			opOK &= this -> digitalWrite(RL1, HIGH);
			opOK &= this -> digitalWrite(RL2, HIGH);
			break;
		case ALL_RELAYS_ON:
			opOK = this -> digitalWrite(RL_COM, HIGH);
			opOK &= this -> digitalWrite(RL1, LOW);
			opOK &= this -> digitalWrite(RL2, LOW);
			break;
		case SET_INPUT_RANGE_25V:
			opOK = this -> digitalWrite(RL_COM, LOW);
			opOK &= this -> digitalWrite(RL2, HIGH);
			break;
		case SET_INPUT_RANGE_250V:
			opOK = this -> digitalWrite(RL_COM, HIGH);
			opOK &= this -> digitalWrite(RL2, LOW);
			break;
		case SELECT_DIFF_MEAS:
			opOK = this -> digitalWrite(RL_COM, LOW);
			opOK &= this -> digitalWrite(RL1, HIGH);
			break;
		case SELECT_SMU:
			opOK = this -> digitalWrite(RL_COM, HIGH);
			opOK &= this -> digitalWrite(RL1, LOW);
			break;	
		default:
			return false;
	}
	delay(10);
	if(rlatched) {
		opOK &= this -> digitalWrite(RL_COM, HIGH);
		opOK &= this -> digitalWrite(RL1, HIGH);
		opOK &= this -> digitalWrite(RL2, HIGH);
	}
	if (opOK)
		return true;
	return false;
}
	
bool PCA9554::SetLeds(uint8_t ledCommand)
{
	bool opOK = false;
	switch (ledCommand)
	{
		
		case DISP_OFF:
			opOK = this -> digitalWrite(LED_POL_P, HIGH);
			opOK &= this -> digitalWrite(LED_POL_N, HIGH);	
			break;
		case DISP_PROBE_POLARITY_POS:
			opOK = this -> digitalWrite(LED_POL_P, LOW);
			opOK &= this -> digitalWrite(LED_POL_N, HIGH);
			break;
		case DISP_PROBE_POLARITY_NEG:
			opOK = this -> digitalWrite(LED_POL_N, LOW);
			opOK &= this -> digitalWrite(LED_POL_P, HIGH);
			break;
		case DISP_INPUT_DIV25:
			opOK = this -> digitalWrite(LED_DIV_25, LOW);
			opOK &= this -> digitalWrite(LED_DIV_250, HIGH);
			break;
		case DISP_INPUT_DIV250:
			opOK = this -> digitalWrite(LED_DIV_250, LOW);
			opOK &= this -> digitalWrite(LED_DIV_25, HIGH);
			break;
		case ALL_LEDS_OFF:
			opOK = this -> digitalWrite(LED_POL_P, HIGH);
			opOK &= this -> digitalWrite(LED_POL_N, HIGH);
			opOK = this -> digitalWrite(LED_DIV_25, HIGH);
			opOK &= this -> digitalWrite(LED_DIV_250, HIGH);;
			break;
		default:
			return false;
	}
	if(opOK)
		return true;
	return false;
}	

