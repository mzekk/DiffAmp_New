/* Based on Oleg Mazurov's code for rotary encoder interrupt service routines for AVR micros
   here https://chome.nerpa.tech/mcu/reading-rotary-encoder-on-arduino/
   and using interrupts https://chome.nerpa.tech/mcu/rotary-encoder-interrupt-service-routine-for-avr-micros/

   This example does not use the port read method. Tested with Nano and ESP32
   both encoder A and B pins must be connected to interrupt enabled pins, see here for more info:
   https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
*/

#include "EncoderRead.h"
// Khởi tạo con trỏ tĩnh
EncoderRead* EncoderRead::instance = nullptr;
EncoderRead::EncoderRead(uint8_t pinA, uint8_t pinB, uint8_t btnPin)
  : _pinA(pinA), _pinB(pinB), _btnPin(btnPin), _counter(0),
    _lastIncReadTime(0), _lastDecReadTime(0), _pauseLength(500), _fastIncrement(10), _longPress(false) {
    instance = this;
}

void EncoderRead::begin() {
    pinMode(_pinA, INPUT_PULLUP);
    pinMode(_pinB, INPUT_PULLUP);
    pinMode(_btnPin, INPUT_PULLUP);
    
    
    attachInterrupt(44, EncoderRead::readEncoder, CHANGE);
    attachInterrupt(43, EncoderRead::readEncoder, CHANGE);
}

void IRAM_ATTR EncoderRead::readEncoder() {
    //log_printf("Pin Intr \n");
    if (instance) {
        static uint32_t lastDebounceTime = 0;
        uint32_t currentTime = micros();
        
        if (currentTime - lastDebounceTime > 200) { // 200 µs debounce
            static uint8_t old_AB = 3;
            static int8_t encval = 0; 
            static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; 

            old_AB <<= 2;
            if (digitalRead(instance->_pinA)) old_AB |= 0x02; 
            if (digitalRead(instance->_pinB)) old_AB |= 0x01; 

            encval += enc_states[(old_AB & 0x0F)]; 

            if (encval > 3) { 
                int changevalue = 1;
                if ((micros() - instance->_lastIncReadTime) < instance->_pauseLength) {
                    changevalue = instance->_fastIncrement * changevalue; 
                }
                instance->_lastIncReadTime = micros();
                instance->_counter += changevalue; 
                encval = 0;
            }
            else if (encval < -3) { 
                int changevalue = -1;
                if ((micros() - instance->_lastDecReadTime) < instance->_pauseLength) {
                    changevalue = instance->_fastIncrement * changevalue; 
                }
                instance->_lastDecReadTime = micros();
                instance->_counter += changevalue; 
                encval = 0;
            }
            
            lastDebounceTime = currentTime;
        }
    }
}

bool EncoderRead::encBtn() {
    static uint32_t lastPressTime = 0;
    static uint32_t lastDebounceTime = 0;
    static bool lastState = false;
    uint32_t currentTime = millis();
    bool currentState = digitalRead(_btnPin) == HIGH;
    if (currentState != lastState) {
        if (currentTime - lastPressTime > 80) { // 50 ms debounce
            lastPressTime = currentTime;
            lastState = currentState;
            return currentState;       
        }
    }
    else {
        lastPressTime = currentTime;
        lastState = currentState;
    }
    return false;
}

bool EncoderRead::encBtnLongPress() {
    static bool is_down = false;
    static unsigned long press_start = 0;
    bool btn = digitalRead(_btnPin) == HIGH;

    if (btn && !is_down) {
        press_start = millis();
        is_down = true;
    }

    if (!btn && is_down) {
        unsigned long duration = millis() - press_start;
        is_down = false;
        if (duration > 800) {
            _longPress = true;
            return true;
        }
    }
    return false;
}

int EncoderRead::getCounter() {
    return _counter;
}
void EncoderRead::resetCounter() {
    _counter = 0;
}

