#pragma once

#define WIFI_SSID                    "Your-ssid"
#define WIFI_PASSWORD               "Your-password"

#define WIFI_CONNECT_WAIT_MAX        (30 * 1000)

#define NTP_SERVER1                  "pool.ntp.org"
#define NTP_SERVER2                  "time.nist.gov"
#define GMT_OFFSET_SEC               (3600 * 8)
#define DAY_LIGHT_OFFSET_SEC         0

/* LCD CONFIG */
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ   (6528000) //(10 * 1000 * 1000)
// The pixel number in horizontal and vertical
#define EXAMPLE_LCD_H_RES            320
#define EXAMPLE_LCD_V_RES            170
#define LVGL_LCD_BUF_SIZE            (EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES)
#define EXAMPLE_PSRAM_DATA_ALIGNMENT 64

/*ESP32S3*/
#define PIN_LCD_BL                   38

#define PIN_LCD_D0                   39
#define PIN_LCD_D1                   40
#define PIN_LCD_D2                   41
#define PIN_LCD_D3                   42
#define PIN_LCD_D4                   45
#define PIN_LCD_D5                   46
#define PIN_LCD_D6                   47
#define PIN_LCD_D7                   48

#define PIN_POWER_ON                 15

#define PIN_LCD_RES                  5
#define PIN_LCD_CS                   6
#define PIN_LCD_DC                   7
#define PIN_LCD_WR                   8
#define PIN_LCD_RD                   9

#define PIN_BUTTON_1                 0
#define PIN_BUTTON_2                 14
#define PIN_BAT_VOLT                 4

#define PIN_IIC_SCL                  17
#define PIN_IIC_SDA                  18

#define PIN_TOUCH_INT                16
#define PIN_TOUCH_RES                21

//******* BOARD IOs CONFIGURATION ********
#define V_I_SELF                     1 // Select Voltage (LOW) or Current (HIGH) for ADC ETEST measurement (AIN1)
#define KEEP_ON                      2 // Maintain battery voltage (SYSTEM-POWER) if HIGH
#define EN_HPWR                      3 // Enable high voltage power supply circuits if HIGH (VPOS/VNEG/+VAMP/-VAMP)
#define SWPB                         10 // Wheel encoder Pushbutton Switch, active high
#define BUZZ_FAULT                   11 // This is a double function IO
                                        // - When ESP32-S3 driven, is the PWM output for the PiezoBuzzer   
                                        // - When ESP32-S3 set this pin to input with pullup, it checks power supply faults when LOW
#define LDAC_CHRG                    12 // This is a double function IO
                                        // - When ESP32-S3 driven, controls the DAC (IC1) latched output update on rising edge
                                        // - When ESP32-S3 set this pin to input with pullup, it checks battery charge status (LOW = Charging)
#define EN_VC                        13 // Enable BJT/MOSFET Test Supply circuit if HIGH   

#define EN_LPWR                      16 // Enable low voltage power supply circuits if HIGH (VANA/VB/+V2p5/-V2p5)
#define ENC_A                        44 // Wheel encoder A pin
#define ENC_B                        43 // Wheel encoder B pin   
#define ENC_SW                       SWPB // Wheel encoder push-button

#define PIN_IIC_SCL                  17
#define PIN_IIC_SDA                  18
//******* USED BY LILYGO ESP321-S3 BOARD FOR THE ON-BOARD DISPLAY ********
#define PIN_TOUCH_INT                16 // Do not use in application unless meant to control the TFT Display
#define PIN_TOUCH_RES                21 // Do not use in application unless meant to control the TFT Display   

#define IO_RESET_CONF   0x10        // All outputs except P4 (CHARGER_OFF)
#define IO_RESET_STATE  0xEF
#define IOEXP_OUTPUT    1
#define IOEXP_INPUT     0
#define DISABLE_CHARGER 0
#define ENABLE_CHARGER  0x10
