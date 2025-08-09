#include "DiffAmp.h"
#include <ADS122C04.h>  // Load the ADS122C04 4-Channel ADC Library
#include "pin_config.h"
#include "PCA9554.h"
#include <MCP4728.h>
#include <MCP4726.h>  // Load the MCP4726 1-Channel DAC Library
#include <MAX17040.h>
#include "..\src\ui\ui.h"
#include "EncoderRead.h"
#include <WiFi.h>
#include "esp_wifi.h"
#include "..\src\ui\lv_setup.h"
#include <Ticker.h>
#include "esp_pm.h"
#include "esp_sleep.h"
//#include <esp_sleep.h>
//#include "esp_lcd_panel_io.h"
//#include "esp_lcd_panel_ops.h"
//#include "esp_lcd_panel_vendor.h"
//extern esp_lcd_panel_handle_t panel_handle;
//extern esp_lcd_panel_io_handle_t io_handle;

const uint32_t standbyTms[] = {STBT_ALWAYS_ON, STBT_1MIN, STBT_2MIN, STBT_3MIN, STBT_2MIN, STBT_5MIN, STBT_10MIN, STBT_15MIN, STBT_20MIN, STBT_30MIN, STBT_1HOUR};

extern uint16_t sleepCheck;

extern volatile uint ctr; 
extern volatile uint t500ms;
extern float vcalc, vdiffMult,vampSet, battChargeLeft, battVoltage;
extern uint16_t dacVals[4], dacValsEEP[4], vdiffNull, avg_cntr;
extern volatile uint16_t changeTabDelay;
extern SFE_ADS122C04 adc;
extern struct_message voltReadings;
extern volatile bool switchPressed; 
extern volatile bool switchLongPressed; 
extern volatile bool switchOffRequest; 
extern volatile bool switchIsPressed; 
extern volatile bool swPressLv; 
extern volatile bool uiActions; 
extern bool connected;
extern bool vin_range; 

extern unsigned char result;
extern Rotary r;
extern EncoderRead encoder;
extern lv_indev_t *encoder_indev;
void encoder_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
extern bool wait_and_show_standby_screen(uint32_t timeout, uint8_t mode);
extern void WriteOptionsToFile(String dataOptions, sysOptions* sysOpt);
extern PCA9554 ioExp;
extern MCP4728 dac4ch;
extern MCP4726 dac1ch;
extern MAX17040 bMon;
extern String ssid;
extern String pass;
extern lv_obj_t* previous_screen;
extern String dataOptions;
extern SFE_ADS122C04 adc; 

extern hw_timer_t *My_timer;

extern uint16_t sleepTimeout;

Ticker ticker4ms;

bool backLightDimmed = false;
extern bool initWiFi_AP(void);
void setupADC(SFE_ADS122C04& adc);
void tickHandler(void);
void checkSleepModeTimeout(sysOptions* sysOpt, uint32_t currentTime);
void checkBacklightDimTimeout(sysOptions* sysOpt, uint32_t currentTime);
void checkTurnOffRequest(sysOptions* sysOpt, uint32_t currentTime);
bool enterSleepMode(sysOptions* sysOpt);
void resumeSleepmode(int8_t backlight);
bool confirm_valid_wakeup(uint32_t debounce_time_ms = 100);
void hwSysCheck(uint32_t currentTime);

bool chargerState = NOT_CHARGING;

bool enterSleepMode(void);
void waitKeyRelease(void);

struct battState {
  float SoC, SoV;
  uint8_t chargeState;
  uint16_t sleepCyclesNo;
  float battChargeHysteresis;
  bool chargerDisabled;
};
//struct battState checkBattState;
battState checkBattState = {0, 0, ACTIVE_MODE, 0, 0, false};

void battChargeTask(sysOptions* sysOpt, battState* lastBattState);

void HwInit(void){
    pinMode(PIN_POWER_ON, OUTPUT);  //triggers the LCD backlight
    pinMode(PIN_LCD_BL, OUTPUT);    // BackLight enable pin

    pinMode(LDAC_CHRG, INPUT);      // LDAC_CHRG pin has dual function, so it must be set as input when detecting charger presence or output when the DAC LDAC function is needed
    pinMode(LDAC_CHRG, INPUT_PULLUP);// By defaul LDAC_CHRG is set as input with pullup to detect charger presence
    pinMode(SWPB, INPUT_PULLUP);
    pinMode(ENC_A, INPUT_PULLUP);
    pinMode(ENC_B, INPUT_PULLUP);
  
    digitalWrite(PIN_POWER_ON, HIGH);
    digitalWrite(PIN_LCD_BL, HIGH); 
  

    gpio_hold_dis((gpio_num_t)PIN_TOUCH_RES);
    //delay(500);

    pinMode(EN_LPWR, OUTPUT);        // Set Power Enable pin as output
    pinMode(EN_HPWR, OUTPUT);
    digitalWrite(EN_LPWR, HIGH);     // Turn on Low Voltage Circuits
    digitalWrite(EN_HPWR, LOW);     // Turn on High Voltage Circuits
    delay(100);
    bMon.getBatterySoC(battChargeLeft);
    bMon.getBatteryVoltage(battVoltage);

    ioExp.digitalWritePort(0xFF); // Set IO Expander to reset state

    ioExp.portMode(IO_RESET_CONF);    // Set IO Expander Configuration
    ioExp.digitalWritePort(IO_RESET_STATE); // Set IO Expander to reset state
    ioExp.SetRelay(ALL_RELAYS_OFF);
    ioExp.SetRelay(SET_INPUT_RANGE_250V);
    ioExp.SetLeds(DISP_PROBE_POLARITY_POS);
    ioExp.SetLeds(DISP_INPUT_DIV250);    
    dacValsEEP[2] = dacVals[2];
    dacValsEEP[3] = dacVals[3];
  
    dac4ch.setVref(INT_VREF_ALL);
    dac4ch.setGain(GAIN_ALL);

    dac1ch.setOptions(VDD_VREF + DAC_GAIN_1X);
    dac1ch.writeDACFast(vdiffNull);
  
    delay(100);
    digitalWrite(EN_HPWR, LOW);     // Turn on High Voltage Circuits
  

    if (adc.begin(ADS122C04_ADC_ID) == false) //Connect to the PT100 using the defaults: Address 0x45 and the Wire port
    {
      serialPrintDebug("ADS122C04 not detected at default I2C address. Please check wiring. Freezing.\n");
      while (1);
    }
    serialPrintDebug("ADS122C04 found, initializing...\n");
    setupADC(adc);
    adc.start();

    ledcSetup(BL_PWM, 2000, 8);
    ledcAttachPin(PIN_LCD_BL, 0);
    ledcWrite(BL_PWM, DEFAULT_BRIGHTNESS);
    
    ledcSetup(BUZZER_PWM, 2500, 8);
    ledcAttachPin(BUZZ_FAULT, 1);
    ledcWrite(BUZZER_PWM, 127);
}

void keepPowerOn(void){
    delay(50);
    pinMode(KEEP_ON, OUTPUT);       // Self Maintain Power Switch ON
    digitalWrite(KEEP_ON, HIGH);    
}

void encoder_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {

    static int32_t last_counter = 0;
  
    int32_t counter = encoder.getCounter();
    //bool btn_state = encoder.encBtn();
    bool btn_state = switchPressed;

    //serialPrintDebug("Button State: %d\n", int(btn_state));
    if(counter != last_counter)
      uiActions = true;
    data->enc_diff = counter - last_counter;
    if(switchIsPressed)
      data->state = LV_INDEV_STATE_RELEASED;
    else data->state = btn_state ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;

    last_counter = counter;
  }


void setupEncoderLV(void){
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_ENCODER;
  indev_drv.read_cb = encoder_read;
  encoder_indev = lv_indev_drv_register(&indev_drv);
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);
  attachInterrupt(ENC_A, EncoderRead::readEncoder, CHANGE);
  attachInterrupt(ENC_B, EncoderRead::readEncoder, CHANGE);
}

void setBacklight(uint8_t levelMax, uint8_t levelMin, uint16_t timeout){
    //char txtd[10];
    //sprintf(txtd, "%d%%", levelMax);
	  //lv_label_set_text(ui_MaxBrghtValLbl, txtd);
    //lv_slider_set_value(ui_MaxBrightnessCtrl, levelMax, LV_ANIM_OFF);
    levelMax = levelMax * BL_PWM_RES / 100;
    ledcSetup(BL_PWM, 2000, 8);
    ledcAttachPin(PIN_LCD_BL, 0);
    ledcWrite(BL_PWM, levelMax);
    gpio_hold_dis((gpio_num_t)PIN_TOUCH_RES);
    //sprintf(txtd, "%d%%", levelMin);
	  //lv_label_set_text(ui_MinBrghtValLbl, txtd);
    //lv_slider_set_value(ui_MinBrightnessCtrl, levelMin, LV_ANIM_OFF);
    //sprintf(txtd, "%d min", timeout);
	  //lv_label_set_text(ui_TimeoutValLbl, txtd);
    //lv_slider_set_value(ui_BacklightToutCtrl, timeout, LV_ANIM_OFF);    
}



void setup_TMR_intrpt(uint16_t intrTime){
    My_timer = timerBegin(0, 80, true);
    timerAttachInterrupt(My_timer, &onTimer, false);
    timerAlarmWrite(My_timer, intrTime, true); // Interrupt timer set to 200us 
    timerAlarmEnable(My_timer); //Just Enable
    analogSetClockDiv(200);
    // Changes for new ESP library version
    //My_timer = timerBegin(1000000);
    //timerAttachInterrupt(My_timer, &onTimer);
    //timerAlarm(My_timer, intrTime, true, 0); // Interrupt timer set to 200us 

}

/*
void IRAM_ATTR onTimer(void){
    if(changeTabDelay > 0)
      --changeTabDelay;
    ++ctr;
    ++t500ms;
    if(digitalRead(SWPB)){
      if(++pbdebounce >= SW_LONG_PRESS){
        pbdebounce = SW_LONG_PRESS;
        switchIsPressed = true;
      }
    }
    else {
      if(pbdebounce >=  SW_LONG_PRESS){
        switchLongPressed = true;
        switchPressed = false;
        swPressLv = false;
        uiActions = true;
      }
      else if(pbdebounce >=  SW_DEBOUNCE){
        switchPressed = true;
        swPressLv = true;
        uiActions = true;
      }
      pbdebounce = 0;
      switchIsPressed = false;
    }
}
*/

void tickHandler() {
  static uint pbdebounce = 0; 
  const uint16_t SW_DEBOUNCE = 8;     // e.g., 32ms
  const uint16_t SW_LONG_PRESS = 250; // e.g., 1s

  if (changeTabDelay > 0)
    --changeTabDelay;

  ++ctr;
  ++t500ms;

  bool sw = digitalRead(SWPB);  // safe to call here

  if (sw) {
    if (++pbdebounce >= SW_LONG_PRESS) {
      pbdebounce = SW_LONG_PRESS;
      switchIsPressed = true;
    }
  } else {
    if (pbdebounce >= SW_LONG_PRESS) {
      switchLongPressed = true;
      switchPressed = false;
      swPressLv = false;
      uiActions = true;
    }
    else if (pbdebounce >= SW_DEBOUNCE) {
      switchPressed = true;
      swPressLv = true;
      uiActions = true;
    }
    pbdebounce = 0;
    switchIsPressed = false;
  }
}

void timings(sysOptions* sysOpt){
  checkBattState.chargeState = ACTIVE_MODE;
  uint32_t currentTime = millis();
  checkSleepModeTimeout(sysOpt, currentTime);      
  checkBacklightDimTimeout(sysOpt, currentTime);
  checkTurnOffRequest(sysOpt, currentTime);
  hwSysCheck(currentTime);
  battChargeTask(sysOpt, &checkBattState);
  if(checkBattState.chargeState == BATT_TOO_LOW){
    // Display Shutdown screen
    while(1)
      digitalWrite(KEEP_ON, LOW);
  }

  if(switchIsPressed == true)
    lv_indev_reset(encoder_indev, NULL);
  uiActions = false;  
}

void checkSleepModeTimeout(sysOptions* sysOptLoc, uint32_t currentTime){
  static uint32_t lastStandbyTime = currentTime;
  if(sysOptLoc->StandbyTout != 0 && uiActions == false && (chargerState & !sysOptLoc->SleepWithCharger) == false){
    if(currentTime - lastStandbyTime > standbyTms[sysOptLoc->StandbyTout]){
      lv_obj_t* prev_screen = lv_scr_act();
      uint8_t hvRegulatorState = digitalRead(EN_HPWR);
      bool userCancelled = enterSleepMode(sysOptLoc);
      if(!userCancelled)
        resumeSleepmode(sysOptLoc->BacklightBrightness);
      waitKeyRelease(); 
      switchPressed = false;
      switchLongPressed = false;
      lastStandbyTime = millis();
      lv_scr_load(prev_screen);  // Restore previous screen
      lv_timer_handler();  // LVGL refresh
      digitalWrite(EN_LPWR, HIGH);
      digitalWrite(EN_HPWR, hvRegulatorState);
      delay(100);
      setupADC(adc);
      adc.start();
    }
  }
  else lastStandbyTime = currentTime;
  
  //serialPrintDebug("Heap: %d   Min: %d\n", ESP.getFreeHeap(), ESP.getMinFreeHeap());
}

void checkBacklightDimTimeout(sysOptions* sysOpt, uint32_t currentTime){
  static uint32_t lastBacklightTime = currentTime;
  if(sysOpt->BacklightTout != 0){
    if(currentTime - lastBacklightTime > sysOpt->BacklightTout * 5000){
      ledcWrite(BL_PWM, sysOpt->BacklightLowBrght* BL_PWM_RES / 100);
      lastBacklightTime = millis();
      backLightDimmed = true;
    }
    if(uiActions || chargerState == CHARGING){
      ledcWrite(BL_PWM, sysOpt->BacklightBrightness * BL_PWM_RES / 100);      
      lastBacklightTime = millis();
      backLightDimmed = false;
    }
  }
  else 
    lastBacklightTime = currentTime; 
}

void checkTurnOffRequest(sysOptions* sysOpt, uint32_t currentTime){
  static uint32_t lastOffRequestTime = currentTime;

  if(digitalRead(SWPB))
    ++lastOffRequestTime;
  else 
    lastOffRequestTime = currentTime;

  if(currentTime - lastOffRequestTime > sysOpt->SwitchTurnOffTime){
    if (wait_and_show_standby_screen(FIVE_SECONDS, OFF_MSG_ID)){ 
      serialPrintDebug("Low Battery Charge — Forcing Shutdown\n");
      delay(100);
      digitalWrite(KEEP_ON, LOW);
      while(true);  
    }  
  }
} 



/**
 * Enters the system into light sleep mode based on the provided system options.
 * 
 * This function handles the transition of the system into a low-power sleep mode,
 * managing power states and peripherals as necessary to conserve energy. It also
 * monitors the battery charge and voltage during the sleep cycles, and forces a shutdown
 * if the battery is below specified thresholds. The function will exit sleep mode if 
 * a user keypress is detected.
 * 
 * @param sysOpt A pointer to the sysOptions structure containing configuration settings 
 *               for sleep mode, including sleep cycle duration, maximum sleep time, and 
 *               battery thresholds.
 * @return True if the sleep mode was canceled by the user, false otherwise.
 */

bool enterSleepMode(sysOptions* sysOpt){
  if (!wait_and_show_standby_screen(FIVE_SECONDS, SLEEP_MSG_ID)) {
        return true;  // Cancelled by user
  }

  WriteOptionsToFile(dataOptions, sysOpt);
  //uint16_t sleepCyclesNo = 0;
  checkBattState.chargeState = SLEEP_MODE;
  checkBattState.battChargeHysteresis = 0;
  checkBattState.chargerDisabled = false;
  checkBattState.sleepCyclesNo = 0;

  digitalWrite(EN_HPWR, LOW);
  serialPrintDebug("Preparing to enter light sleep...\n");
  esp_lcd_panel_io_tx_param(io_handle, 0x10, NULL, 0);  // SLPIN (0x10)

  //timerAlarmDisable(My_timer);
  ioExp.SetLeds(ALL_LEDS_OFF);
  serialPrintDebug("Turn off Display\n");
  Wire.end();
  // Disconnect Wi-Fi
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();
  serialPrintDebug("Turn off WIFI\n");

  //turn_off_display backlight;
  ledcWrite(BL_PWM, 0);
  digitalWrite(PIN_POWER_ON, LOW);
  // esp_sleep_enable_timer_wakeup(sysOpt->SleepTimeCycleMs * 1000ULL);
  //serialPrintDebug("Entering RTC light sleep now. RTC wakeup time: %d ms, no of cycles: %d\n", sysOpt->SleepTimeCycleMs, sysOpt->SleepNumCyclesToMeas);
  
  serialPrintDebug("Actual CPU Frequency: %dMHz\n", getCpuFrequencyMhz());
  setCpuFrequencyMhz(10);
  serialPrintDebug("New CPU Frequency: %dMHz\n", getCpuFrequencyMhz());
 
  while(true){
    // Stop I2C

    digitalWrite(EN_LPWR, LOW);    
    //esp_light_sleep_start();  // Sleep... 
    delay(sysOpt->SleepTimeCycleMs); // Single sleep cycle (simulated in this case)

    //esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    //if ((cause == ESP_SLEEP_WAKEUP_TIMER && ++sleepCyclesNo == sysOpt->SleepNumCyclesToMeas) || --maxNumberofSleep == 0) {
    Wire.begin(PIN_IIC_SDA, PIN_IIC_SCL); 
    delay(10);
    
    battChargeTask(sysOpt, &checkBattState);
    if(checkBattState.chargeState == SLEEP_TIME_EXPIRED || checkBattState.chargeState == BATT_TOO_LOW)
      while(1)
        digitalWrite(KEEP_ON, LOW);
    else {
          if(digitalRead(SWPB) == HIGH){
            delay(20);
            if(digitalRead(SWPB) == HIGH){
              setCpuFrequencyMhz(240);
              serialPrintDebug("Exit Sleep by keypress\nNew CPU Frequency: %dMHz\n", getCpuFrequencyMhz());
              ioExp.portMode(ENABLE_CHARGER); // Enable the charger
              checkBattState.chargeState = ACTIVE_MODE;
              checkBattState.battChargeHysteresis = 0;
              return false;
            }
          }      
      }  
    Wire.end();  
  }
}



void battChargeTask(sysOptions* sysOpt, battState* lastBattState) {
  #define CHARGER_OFF_STATE 0x01
  #define CHARGER_WAS_DISABLED 0x02
  #define CHARGER_RESTORED 0x04
  #define CHARGER_ON_STATE 0x08
 
  static bool sleepCycleToggle = false, testChargerWhenDisabled = false;
  static uint16_t sleepTime = 0, status = 0, oldStatus;
  static uint32_t maxNumberofSleep = sysOpt->SleepMaxTime * MS_IN_ONE_HOUR / sysOpt->SleepTimeCycleMs;  
  float MaxBattCharge = MAX_BATT_CHARGE;
  if(sysOpt->BattProtect)
    MaxBattCharge = SAFE_BATT_CHARGE;  
  bMon.getBatterySoC(lastBattState->SoC);
  bMon.getBatteryVoltage(lastBattState->SoV);
  //lastBattState.chargeState = lastBattState->chargeState;
  bool chargerPortState = bool(digitalRead(LDAC_CHRG));

  uint32_t currentTime = millis();
  static uint32_t lastChargergPresenceTime = currentTime, retestChargerTime = currentTime;
  static bool lastChargerPortState = !chargerPortState;
  static bool lastChargerState = lastChargerPortState;
  static bool evaluateChargerState = false;
  //static uint8_t lastChargerState = CHARGER_OFF_STATE;

  if(testChargerWhenDisabled && currentTime - retestChargerTime > 9500){
    if(digitalRead(LDAC_CHRG) == HIGH)
      lastBattState->chargerDisabled = false;          
    else 
      ioExp.portMode(DISABLE_CHARGER); // Turn off the charger   
    serialPrintDebug("Tested Charger Presence: %d\n", int(digitalRead(LDAC_CHRG)));  
    testChargerWhenDisabled = false;  
    retestChargerTime = currentTime;
    evaluateChargerState = true;
  }
  
  switch (lastBattState->chargeState)
  {
    // When operating in normal mode, the battery charge is monitored
    // if the charge is above the max threshold, the charge cycle is stopped,
    // this is shown on the display by the GREEN charging icon.
    // When the battery is being charger, the charging icon become YELLOW
    // When the charger is disconnected, the charging icon become idle (DARK GRAY)
    case ACTIVE_MODE:
      // When the charger is disabled, the testChargerWhenDisabled flag is set periodically 
      // to test if the charger is still present
      
    
      if(chargerPortState != lastChargerPortState || evaluateChargerState){ 
        if(currentTime - lastChargergPresenceTime > 1000) { 
          lastChargerPortState = bool(digitalRead(LDAC_CHRG));
          lastChargergPresenceTime = currentTime;
          if(lastChargerPortState == false){ // Battery is charging when LDAC_CHRG is low
            serialPrintDebug("Charger Presence Detected\n");
            lv_obj_set_style_img_recolor(ui_ChargingImg, lv_color_hex(0xF7F039), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor_opa(ui_ChargingImg, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            //chargerState = CHARGING ;
          }
          else if(lastBattState->chargerDisabled == true){
            serialPrintDebug("Charger was Disabled to protect battery from overcharging\n");
            lv_obj_set_style_img_recolor(ui_ChargingImg, lv_color_hex(0x36F415), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor_opa(ui_ChargingImg, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            chargerState = NOT_CHARGING;
          }
          else {
            serialPrintDebug("Charger Removed \n");
            lv_obj_set_style_img_recolor(ui_ChargingImg, lv_color_hex(0x1E1E02), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor_opa(ui_ChargingImg, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            chargerState = NOT_CHARGING;
          }
        }   
        evaluateChargerState = false;    
      }
      else chargerPortState = lastChargerPortState;
      //else 
      //  lastChargergPresenceTime = currentTime;

      if(chargerPortState == HIGH){   
          status = CHARGER_OFF_STATE;
          // Check weather the charger is enabled or not
          if (lastBattState->chargerDisabled == true){   
            //serialPrintDebug("AM with Charger was Disabled\n");
            status |= CHARGER_WAS_DISABLED;  
            if(currentTime - retestChargerTime > 9000){ // Battery is not charging when LDAC_CHRG is low
              //retestChargerTime = currentTime;
              ioExp.portMode(ENABLE_CHARGER); // Enable the charger
              testChargerWhenDisabled = true;
            }
            if(lastBattState->SoC < (MaxBattCharge - lastBattState->battChargeHysteresis)){
              ioExp.portMode(ENABLE_CHARGER); // Enable the charger
              //lastBattState->chargerDisabled = false;
              lastBattState->battChargeHysteresis = 0;
              status |= CHARGER_RESTORED;
              //serialPrintDebug("Max charge reached: %f\n", lastBattState->SoC);
            }
          } 
                
          // Check if battery charge or voltage is too low to guarantee operational conditions -> Turn off power
          if(lastBattState->SoC < sysOpt->BattMinChargeLeft || lastBattState->SoV < sysOpt->BattLowThreshold) {  
            serialPrintDebug("Batt too low - Charge Left: %f, Voltage: %f", lastBattState->SoC, lastBattState->SoV);
            lastBattState->chargeState = BATT_TOO_LOW;         
          }       
        }
        else if(testChargerWhenDisabled == false) {
          status = CHARGER_ON_STATE;
          //serialPrintDebug("AM Charger Detected. Charge: %f\n", lastBattState->SoC);  
          //lastBattState.chargeState = ACTIVE_CHARGING;
          if(lastBattState->SoC > MaxBattCharge){            
            ioExp.portMode(DISABLE_CHARGER); // Turn off the charger   
            lastBattState->chargerDisabled = true;
            lastBattState->battChargeHysteresis = BATTERY_RECHARGE_HYSTERESIS;
            retestChargerTime = currentTime;
            testChargerWhenDisabled = false;
            //serialPrintDebug("Restaring charge: %f\n", lastBattState->SoC);
            status |= CHARGER_WAS_DISABLED;
          }        
        }
      if(status != oldStatus){
        switch(status){
          case 0:
            serialPrintDebug("(A) Sampling Charger pin: %f\n", lastBattState->SoC);
          case CHARGER_OFF_STATE:
            serialPrintDebug("(A) Charger OFF - Charge State: %f\n", lastBattState->SoC);
          break;
          case CHARGER_OFF_STATE + CHARGER_WAS_DISABLED:
            serialPrintDebug("(A) Charger was Disabled - Charge State: %f\n", lastBattState->SoC);
            break;
          case CHARGER_OFF_STATE + CHARGER_RESTORED + CHARGER_WAS_DISABLED:
            serialPrintDebug("(A) Charger Restored - Charge State: %f\n", lastBattState->SoC);
            break;  
          case CHARGER_ON_STATE:
            serialPrintDebug("(A) Charger ON - Charge State: %f\n", lastBattState->SoC);
          break;
          case CHARGER_ON_STATE + CHARGER_WAS_DISABLED:
            serialPrintDebug("(A) Charger is now Disabled - Charge State: %f\n", lastBattState->SoC);
            break;  
        }
      }
      oldStatus = status;
    break;
    
    // When operating in SLEEP mode, the battery is monitored as in NORMAL mode
    // the only difference is that the icon is not updated since the display is off
    case SLEEP_MODE:    
      if(chargerPortState == HIGH){   
        status = CHARGER_OFF_STATE;
        //lastBattState.chargeState = SLEEP_NOT_CHARGING;
        ioExp.SetLeds(ALL_LEDS_OFF);
        // Check if max number of sleep time has been reached
        if (--maxNumberofSleep == 0 ){
          setCpuFrequencyMhz(240);
          serialPrintDebug("Max Sleep Time Reached\nCPU Frequency: %dMHz\n", getCpuFrequencyMhz());
          lastBattState->chargeState = SLEEP_TIME_EXPIRED; // Force system power off
        }  
        // Check weather the charger is enabled or not
        if (lastBattState->chargerDisabled == true){    
          status |= CHARGER_WAS_DISABLED;             
          if(lastBattState->SoC < (MaxBattCharge - lastBattState->battChargeHysteresis)){
            ioExp.portMode(ENABLE_CHARGER); // Enable the charger
            lastBattState->chargerDisabled = false;
            lastBattState->battChargeHysteresis = 0;
            status |= CHARGER_RESTORED;
          }
        }      
        // Check if it is time to test the battery charge state
        checkBattState.sleepCyclesNo = checkBattState.sleepCyclesNo + 1;    
        if (checkBattState.sleepCyclesNo >= sysOpt->SleepNumCyclesToMeas){              
          checkBattState.sleepCyclesNo = 0;
          if(lastBattState->chargerDisabled == true){
            ioExp.portMode(ENABLE_CHARGER); // Enable the charger for testing charger is still connected
            ioExp.SetLeds(DISP_PROBE_POLARITY_POS);  // Display battery monitor sampling with charger disconnected    
            testChargerWhenDisabled = true;
            retestChargerTime = currentTime + 9000;
          }
          // Display battery monitor sampling with charger connected (charge paused)
          else ioExp.SetLeds(DISP_PROBE_POLARITY_NEG);
          ioExp.SetLeds(ALL_LEDS_OFF);
          // Check if battery charge or voltage is too low to guarantee operational conditions -> Turn off power
          if(lastBattState->SoC < sysOpt->BattMinChargeLeft || lastBattState->SoV < sysOpt->BattLowThreshold) {  
            lastBattState->chargeState = BATT_TOO_LOW;        
          }   
        }
        // Check if the battery charge is below threshold and need recharging
      }
      else {
        status = CHARGER_ON_STATE;
        //lastBattState.chargeState = SLEEP_CHARGING;
        maxNumberofSleep = sysOpt->SleepMaxTime * MS_IN_ONE_HOUR / sysOpt->SleepTimeCycleMs;   

        if(lastBattState->SoC > MaxBattCharge){     
          status |= CHARGER_WAS_DISABLED;       
          ioExp.portMode(DISABLE_CHARGER); // Turn off the charger   
          lastBattState->chargerDisabled = true;
          lastBattState->battChargeHysteresis = BATTERY_RECHARGE_HYSTERESIS;
          ioExp.SetLeds(ALL_LEDS_OFF);  
        }  
        else if(sleepCycleToggle)
          ioExp.SetLeds(DISP_PROBE_POLARITY_POS);  // Display charger is connected and charging  
        else ioExp.SetLeds(ALL_LEDS_OFF);
          
        checkBattState.sleepCyclesNo = 0;
        sleepCycleToggle = !sleepCycleToggle;
      }
      if(status != oldStatus){
        switch(status){
          case 0:
            serialPrintDebug("(A) Sampling Charger pin: %f\n", lastBattState->SoC);
          case CHARGER_OFF_STATE:
            serialPrintDebug("(SLP) Charger OFF - Charge State: %f\n", lastBattState->SoC);
          break;
          case CHARGER_OFF_STATE + CHARGER_WAS_DISABLED:
            serialPrintDebug("(SLP) Charger was Disabled - Charge State: %f\n", lastBattState->SoC);
            break;
          case CHARGER_OFF_STATE + CHARGER_RESTORED + CHARGER_WAS_DISABLED:
            serialPrintDebug("(SLP) Charger Restored - Charge State: %f\n", lastBattState->SoC);
            break;  
          case CHARGER_ON_STATE:
            serialPrintDebug("(SLP) Charger ON - Charge State: %f\n", lastBattState->SoC);
          break;
          case CHARGER_ON_STATE + CHARGER_WAS_DISABLED:
            serialPrintDebug("(SLP) Charger is now Disabled - Charge State: %f\n", lastBattState->SoC);
            break;  
        }
      }
      oldStatus = status;
    break;
  
    default:
      serialPrintDebug("Unknown state\n");
    break;
  }
}
  


bool confirm_valid_wakeup(uint32_t debounce_time_ms) {
  uint32_t start = millis();
  if (digitalRead(SWPB) == LOW)
        return false;  // Already released — likely a glitch

  while ((millis() - start) < debounce_time_ms) {
      if (digitalRead(SWPB) == LOW)
            return false;  // Released too early
      delay(5);  // Small delay for stability
    }

  return true;  // Valid press
}  

void resumeSleepmode(int8_t backlight) {
  //timerAlarmEnable(My_timer); //Just Enable
  delay(100);
  //Serial.end();    // Optional
  //Serial.begin(115200);
  digitalWrite(PIN_POWER_ON, HIGH);
  Wire.end();
  Wire.begin(PIN_IIC_SDA, PIN_IIC_SCL);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  //serialPrintDebug("Setting WiFi ssid:%s, pass:%s\n", ssid, pass);

  if(vin_range == DIV_250)
    ioExp.SetLeds(DISP_INPUT_DIV250);
  else 
    ioExp.SetLeds(DISP_INPUT_DIV25);  

  esp_lcd_panel_io_tx_param(io_handle, 0x11, NULL, 0);  // SLPOUT (0x11)
  //vTaskDelay(pdMS_TO_TICKS(120));  // Datasheet: wait 120ms
  //serialPrintDebug("Setting Display\n");
  ledcWrite(BL_PWM, backlight);
  //lv_indev_reset(NULL, NULL);
  //attachInterrupt(ENC_A, EncoderRead::readEncoder, CHANGE);
  //attachInterrupt(ENC_B, EncoderRead::readEncoder, CHANGE);
  //serialPrintDebug("Setting Backlight:%d\n", backlight);
  initWiFi_AP();
  connected = false;
  //WiFi.mode(WIFI_STA);
  //esp_wifi_start();
  //WiFi.begin(ssid.c_str(), pass.c_str());  // You must manage these
}

void waitKeyRelease(void) { 
  bool keyPressed = true;
  uint8_t keyDebounce = 0;
  while (keyPressed) {
      if (digitalRead(SWPB) == HIGH) {
        keyDebounce = 0;
      }
      else if(++keyDebounce >= SW_RELEASE_DEBOUNCE) 
        keyPressed = false;  // ++keyDebounce
      delay(1);  
  }  
}
/**
 * @brief Interrupt handler for a pin change.
 * 
 * This function is triggered by a pin interrupt and processes the rotary encoder.
 */

void hwSysCheck(uint32_t currentTime) {
  // Reading the board temperature
  static uint32_t lastTempTime = currentTime;
  char tempStr[10];
  if(currentTime - lastTempTime > TEN_SECONDS_TEMP_READ){ 
    lastTempTime = currentTime;
    float internalTemp = adc.readInternalTemperature();  
    sprintf(tempStr, "%.1f°C", internalTemp);
    lv_label_set_text(ui_SysInfoLbl, tempStr);
    setupADC(adc);
    adc.start();
  }
}

void setupADC(SFE_ADS122C04& adcObj) {
    adcObj.setInputMultiplexer(ADS122C04_MUX_AIN0_AIN2); // Route AIN0 and AIN2 to AINP and AINN
    adcObj.setGain(ADS122C04_GAIN_1); // Set the gain to 1
    adcObj.enablePGA(ADS122C04_PGA_DISABLED); // Disable the Programmable Gain Amplifier
    adcObj.setDataRate(ADS122C04_DATA_RATE_20SPS); // Set the data rate (samples per second) to 20
    adcObj.setOperatingMode(ADS122C04_OP_MODE_NORMAL); // Disable turbo mode
    adcObj.setConversionMode(ADS122C04_CONVERSION_MODE_CONTINUOUS); // Use single shot mode
    adcObj.setVoltageReference(ADS122C04_VREF_INTERNAL); // Use the internal 2.048V reference
    adcObj.enableInternalTempSensor(ADS122C04_TEMP_SENSOR_OFF); // Disable the temperature sensor
    adcObj.setDataCounter(ADS122C04_DCNT_DISABLE); // Disable the data counter (Note: the library does not currently support the data count)
    adcObj.setDataIntegrityCheck(ADS122C04_CRC_DISABLED); // Disable CRC checking (Note: the library does not currently support data integrity checking)
    adcObj.setBurnOutCurrent(ADS122C04_BURN_OUT_CURRENT_OFF); // Disable the burn-out current
    adcObj.setIDACcurrent(ADS122C04_IDAC_CURRENT_OFF); // Disable the IDAC current
    adcObj.setIDAC1mux(ADS122C04_IDAC1_DISABLED); // Disable IDAC1
    adcObj.setIDAC2mux(ADS122C04_IDAC2_DISABLED); // Disable IDAC2
  }


float updateVdiff(float raw_ADC_data, uint16_t sampleAverages){
  if(adc.checkDataReady() == true) {
    //int32_t raw_ADC_data = adc.readADC();
    float fastSample= adc.readRawVoltage();
    char txtd[10];
    voltReadings.vse = fastSample;
    raw_ADC_data += fastSample;
    fastSample = vdiffInCalc(fastSample, vdiffMult);
    if(fastSample < 0){
        ioExp.SetLeds(DISP_PROBE_POLARITY_NEG);
        fastSample = -fastSample;
        lv_label_set_text(ui_VdiffSignLbl, "-");
    }    
    else {
      ioExp.SetLeds(DISP_PROBE_POLARITY_POS);
      lv_label_set_text(ui_VdiffSignLbl, "+");
    }
    uint8_t barVinValpc = uint8_t(fastSample * 100 / (6.25*vdiffMult));
    lv_bar_set_value(ui_VdiffBarVal, barVinValpc, LV_ANIM_OFF);
    if(++avg_cntr == sampleAverages) {
      avg_cntr = 0;
      raw_ADC_data /= sampleAverages;
      fastSample = vdiffInCalc(raw_ADC_data, vdiffMult);
      voltReadings.vdiff = fastSample;
      sprintf(txtd, "%+7.4fV", fastSample);
      lv_label_set_text(ui_VdiffMeasVal, txtd);
      //serialPrintDebug("%s\n", txtd);
      raw_ADC_data = 0;
    }   
  }
  return raw_ADC_data;
}
  
float vdiffInCalc(int32_t raw_ADC_data, float vinDiv){
    float voffset = 0.00; 
    // float diffInV = ((float)raw_ADC_data) * 244.14e-9;
    // diffInV = diffInV + R31 * (diffInV * R36 - R35 * (ADS122C04_VREF - diffInV)) / (R35 * R36);
    float diffInV = ((float)raw_ADC_data) * -3.1426 * 244.14e-9 - voffset;
    return diffInV * vinDiv;
}

void setRelayDivider(bool state){
  if(state == true){
    ioExp.SetRelay(SET_INPUT_RANGE_25V);
    ioExp.SetLeds(DISP_INPUT_DIV25);
    lv_checkbox_set_text(ui_VinDividerBox, "Vin/250");
    vdiffMult = 10;
  }
  else {
    ioExp.SetRelay(SET_INPUT_RANGE_250V);
    ioExp.SetLeds(DISP_INPUT_DIV250);
    lv_checkbox_set_text(ui_VinDividerBox, "Vin/25");
    vdiffMult = 1.0;
  }
}

void enableHV_Regs(float vset){
  digitalWrite(EN_HPWR, HIGH);
  delay(1000);
  vampSet = VAMP_MIN;
  while (vampSet <= vset) {
    delay(30);
    vcalc = dac4ch.vampp2vdac(vampSet);
    dacVals[2] = dac4ch.float2dac(vcalc);
    vcalc = dac4ch.vampn2vdac(-vampSet);
    dacVals[3] = dac4ch.float2dac(vcalc);
    dac4ch.writeAllDACsFast(dacVals);
    dac4ch.updateDACs();
    if(vampSet == vset) 
      break;
    if((vset - vampSet) < VAMP_STEP)
      vampSet = vset;
    else vampSet += VAMP_STEP;  
  }
}



