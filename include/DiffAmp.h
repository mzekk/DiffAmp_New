#ifndef DiffAmp_h
#define DiffAmp_h
#include <Arduino.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
//#include <Rotary.h>


//**** I2C SLAVE ADDRESSES ****
#define MAX17040G_FG_ID   0x36  // Battery Fuel Gauge
#define PCA9554_PE_ID     0x20  // Port expander
#define ADS122C04_ADC_ID  0x40  // 24-Bit Analog to Digital Converter
#define MCP4728_4DAC_ID   0x60  // 4-Channel 12-Bit DAC
#define MCP4726A3_1DAC_ID 0x63  // 1-Channel 12-Bit DAC

#define BL_PWM            0     // Backlight PWM channel
#define BUZZER_PWM        1     // Buzzer PWM channel

#define BL_PWM_RES        255   // Backlight PWM resolution
#define INTR_TIME         4000  // Interrupt timer in microseconds

#define TIME500ms         500000/INTR_TIME
//#define SW_DEBOUNCE       60000/INTR_TIME //SW Debounce
#define SW_TURN_OFF       3000  //SW Request to turn off in milliseconds
//#define SW_LONG_PRESS     800000/INTR_TIME  //SW Long Press
#define TAB_DELAY         1000000/INTR_TIME
#define SW_RELEASE_DEBOUNCE 50

#define MS100             100
#define STBT_ALWAYS_ON    0
#define STBT_1MIN         20000
#define STBT_2MIN         120000
#define STBT_3MIN         180000
#define STBT_5MIN         300000
#define STBT_10MIN        600000
#define STBT_15MIN        900000
#define STBT_20MIN        1200000
#define STBT_30MIN        1800000
#define STBT_1HOUR        3600000

#define MS_IN_ONE_HOUR    3600000
#define FIVE_SECONDS      5
#define TEN_SECONDS_TEMP_READ  10000
#define LOW_POWER_TIME  5000/20  // Low power timeout in ms, 1000 accounts for the delay in the sleep loop
#define SLEEP_INTERVAL_MILLISEC 500         // RTC wake interval in ms
#define SLEEP_CYCLES_TO_MEASURE  20                // Number of sleep cycles to measure the battery voltage and charge status (tsample = SLEEP_INTERVAL_MILLISEC * MAX_SLEEP_CYCLES)
#define SLEEP_MSG_ID      0
#define OFF_MSG_ID        1


#define R31                10000.0  // Rtop ADC input #0
#define R35                1580.0  // Rbot ADC input #0
#define R36                1330.0  // Rref ADC input #0
#define ADS122C04_VREF     2.048   // ADC reference voltage
#define VAMP_MIN           5.0     // Minimum voltage for the amplifier
#define VAMP_MAX           15.0    // Minimum voltage for the amplifier
#define VAMP_DEFAULT       13.0    // Maximum voltage for the amplifier
#define VAMP_STEP          0.1    // Step size for the voltage

#define DIV_250            false
#define DIV_25             true

#define ACTIVE_MODE 1
#define ACTIVE_CHARGING 0xA0
#define ACTIVE_NOT_CHARGING 0xB0
#define SLEEP_MODE 0x10
#define SLEEP_CHARGING 0xA0
#define SLEEP_NOT_CHARGING 0xB0
#define BATT_TOO_LOW 0xFF
//#define SLEEP_TIME_EXPIRED 0xFE

#define CHARGING           true
#define NOT_CHARGING       false
#define LOWBATT_MIN_CHARGE  4  // Min battery charge in percent to guarantee operational conditions
#define LOWBATT_MIN_VOLTAGE 3.4  // Min battery voltage in volts to guarantee operational conditions
#define SLEEP_ALLOWED      false
#define SLEEP_NOT_ALLOWED  true
#define MAX_BATT_CHARGE    97         // Max battery charge in percent to stop the charge cycle    
#define SAFE_BATT_CHARGE   85         // Max battery charge in percent to stop the charge cycle with the Battery Protection option set.   
#define BATTERY_RECHARGE_HYSTERESIS 0.5 // Value (%) subtracted from MAX_BATT_CHARGE or SAFE_BATT_CHARGE when battery is considered charged to restart a new charge cycle

#define DEFAULT_BRIGHTNESS 80

// Values returned by 'process'
// No complete step yet.
#define DIR_NONE 0x0
// Clockwise step.
#define DIR_CW 0x10
// Counter-clockwise step.
#define DIR_CCW 0x20


// RESISTANCE SELECTOR
#define R_AUTO             0
#define R_1                1
#define R_1K               2
#define R_10K              3
#define R_100K             4
#define R_1M               5   
#define R_10M              6
#define R_1M_HV            7
#define R_10M_HV           8
#define RES_HV_VLIM       10.0

// DIODE SELECTOR
#define LOWVF_DIODE        0
#define LED_DIODE          1
#define ZENER_DIODE        2

#define DEBUG_MSG PRINT_DEBUG_MESSAGES

void keepPowerOn(void);
void HwInit(void);
float updateVdiff(float raw_ADC_data, uint16_t sampleAverages);
void setBacklight(uint8_t levelMax, uint8_t levelMin, uint16_t timeout);
void setup_TMR_intrpt(uint16_t);
void IRAM_ATTR onTimer(void);
void IRAM_ATTR pinIntr();
void setRelayDivider(bool);
float vdiffInCalc(int32_t vinconv, float vdiffMult);
void serialPrintDebug(const char * message, ...);

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    float vdiff;
    float vbatt;
    float vse;
} struct_message;

typedef struct {
    float voltSet;
    float currSet;
} vi_limits;

#endif



