
#include <Arduino.h>
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#error "The current version is not supported for the time being, please use a version below Arduino ESP32 3.0"

#endif
#include "ui/ui.h"
#include "lv_conf.h"
#include "lvgl.h" /* https://github.com/lvgl/lvgl.git */
#include "Arduino.h"
#include "Wire.h"
#include "pin_config.h"
#include "DiffAmp.h"
#include "ui/lv_setup.h"
#include "OneButton.h"
#include <PCA9554.h>  // Load the PCA9554 IO Expander Library
#include <MAX17040.h> // Load the MAX17040 Battery Monitor Library
#include <MCP4728.h>  // Load the MCP4728 4-Channel DAC Library
#include <MCP4726.h>  // Load the MCP4726 1-Channel DAC Library
#include <ADS122C04.h> // Load the ADS122C04 4-Channel ADC Library
#include <FileFun.h>
#include <string.h>
#include "EncoderRead.h"
#include <TouchDrvCSTXXX.hpp>
#include "Ticker.h"
#include <WiFi.h>
#include "esp_wifi.h"
#include "SysOptions.h"
#include "BatteryManager.h"


#define TEST_ENCODER 1

#define RE_GND 1
#define RE_COM 3
#define ENCB 2
#define ENCA 11
#define PROBE 12

PCA9554 ioExp(PCA9554_PE_ID);  // Create an object at this address
MAX17040 bMon(MAX17040G_FG_ID);// Create an object at this address
MCP4728 dac4ch(MCP4728_4DAC_ID, VREF_CONF); // Create an object at this address
MCP4726 dac1ch(MCP4726A3_1DAC_ID, INT_VREF_SEL_DAC1, DAC_GAIN_1X); // Create an object at this address
SFE_ADS122C04 adc;
EncoderRead encoder(ENC_A, ENC_B, ENC_SW); //PinA, PinB, button (PinA and PinB must be connected to interrupt-supported pins).

const char* optionsPath = "/MemOptionsDiffAmp.csv";
const char* optionsTestSave = "/MemSave.csv";
struct sysOptions sysOpt;
struct_message incomingReadings;
struct_message voltReadings;
BatteryManager* pBatteryManager = nullptr;

String dataOptions;
String macAddress = "FF:FF:FF:FF:FF:FF";
String macAddressMaster = "24:62:AB:F5:01:48";  
uint8_t masterMacAddress[6], currentScreen;
String espNowSendStatus;
// Define variables to store incoming readings
float incomingVdiff;
float incomingVbatt;
float incomingVSE;
lv_indev_t *encoder_indev = NULL;
lv_group_t *group[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
lv_group_t *group_tabs = NULL;
lv_obj_t* previous_screen;
enum FocusMode { MODE_WIDGETS, MODE_TABS };
FocusMode current_mode = MODE_WIDGETS;

volatile bool homePressed = false; 
volatile bool uiActions = true;

lv_style_t focus_style;

void init_focus_style();
void setupEncoderLV(void);
bool getOptionsfromFile(const char *fileName, sysOptions &options); // Already updated, good.
bool WriteOptionsToFile(const char *fileName, const sysOptions &options); // Already updated, good.
void initUIdata(sysOptions* sysOpt);
void ui_create_groups(void);
uint8_t get_current_screen(uint8_t currentScreen, lv_obj_t* previous_screen);
uint8_t handleScreen(uint8_t screen, lv_obj_t *focused_obj);
void timings(sysOptions* sysOpt);

vi_limits initUI_OhmData(sysOptions* sysOpt, uint16_t range);
vi_limits initUI_DiodeData(sysOptions* sysOpt, uint16_t range);

vi_limits ohm_vi_Limits;
vi_limits diode_vi_Limits;

hw_timer_t *My_timer = NULL;
extern Ticker ticker4ms;
extern void tickHandler(void);
extern void checkSleepModeTimeout(sysOptions* sysOpt, uint32_t currentTime);

void enableHV_Regs(float vset);

// To be removed Functions:
void changeHV(void);
void updateVbatt(void);
void pbHV_Control(void);

uint16_t sleepCheck = 0;

unsigned char result, result_slow = false, avg_cntr = 0;
char time_str[30], date_str[12], time2_str[10];
int32_t raw_ADC_data = 0;
bool connected = false, inProgressWiFi = false;

bool flags_sleep = false, vin_range = DIV_250; 
volatile bool switchPressed = false;
volatile bool switchLongPressed = false; 
volatile bool switchOffRequest = false; 
volatile bool switchIsPressed = false;
volatile bool swPressLv = false;
uint16_t dacVals[4] = {0x7ff, 0x7c0, 0x4f1, 0x4ff}, dacValsEEP[4] = {0x7ff, 0x7c0, 0x4f1, 0x4ff}, vdiffNull = 0x400, iset = 0x7C0;
volatile uint ctr = 0; 
volatile uint t500ms = 0;
uint16_t battpc = 20;
float dispVolt = 5.528, batValue, vampSet = 8.0, vcalc, vdiffMult = 1.0, battChargeLeft, battVoltage;
char txtd[10];
uint8_t adc_ch_sel = ADS122C04_MUX_AIN0_AVSS;
uint16_t sampleAverages = 1;
enum {voltPage, ohmPage, diodePage, smuPage, optionsPage};  // Enumerate the pages of the UI
uint16_t sleepTimeout;

esp_reset_reason_t reset_reason;

void setup()
{
  Serial.begin(115200);
  reset_reason = esp_reset_reason();
  serialPrintDebug("Reset reason: %d\n", reset_reason);
  keepPowerOn();    //Self-maintain power switch
  lv_begin();
  HwInit();
  initLittleFS();
  //inProgressWiFi = initWiFi();    
  WiFi_Service();
  //setup_TMR_intrpt(INTR_TIME);
  ticker4ms.attach_ms(4, tickHandler);
  //sleepModeTest();

  ui_init(); 
  init_focus_style();  
  
  // Load options from file. If it fails, the function will load PredefinedOptions internally.
  if (!getOptionsfromFile(optionsPath, sysOpt)) {
    // If loading failed (e.g., checksum error or new device), save the defaults back to the file system.
    WriteOptionsToFile(optionsPath, sysOpt);;
    serialPrintDebug("Supposed to Write Options");
  }
  pBatteryManager = new BatteryManager(&sysOpt);
  pBatteryManager->enterActiveState(true);

  serialPrintDebug("After Loading Options: Heap: %d   Min: %d\n", ESP.getFreeHeap(), ESP.getMinFreeHeap());
  serialPrintDebug("Ohm Test Voltage: %f, Current: %f\n", ohm_vi_Limits.voltSet, ohm_vi_Limits.currSet);
  serialPrintDebug("Diode Test Voltage: %f, Current: %f\n", diode_vi_Limits.voltSet, diode_vi_Limits.currSet);
  setBacklight(sysOpt.BacklightBrightness, sysOpt.BacklightLowBrght, sysOpt.BacklightTout);
  
  initUIdata(&sysOpt);

  //ohm_vi_Limits = initUI_OhmData(&sysOpt, sysOpt.Ohm_Range);
  //diode_vi_Limits = initUI_DiodeData(&sysOpt, sysOpt.DiodeType);
  setupEncoderLV();
  ui_create_groups();
  
  sleepTimeout = 0;

  t500ms = 0;
  currentScreen = sysOpt.LastScreen;
  lv_tabview_set_act(ui_MainPageTabs, currentScreen, LV_ANIM_OFF); // Show Tab 1 on startup  
  switchOffRequest = false;
  serialPrintDebug("Before Main Loop: Heap: %d   Min: %d\n", ESP.getFreeHeap(), ESP.getMinFreeHeap());
}

int16_t Counter = 0, updDacsCnt = 0;

void loop()
{
  static bool skipMainLoop = false;
  lv_handler();  // Update UI
  t500ms = checkTime(&connected, t500ms); 
    
  uint8_t oldScreen = currentScreen;
  previous_screen = lv_scr_act();
  currentScreen = get_current_screen(currentScreen, previous_screen);
  //serialPrintDebug("Group: %d\n", currentScreen);
  lv_obj_t *focused_obj = lv_group_get_focused(group[currentScreen]);
  if(current_mode == MODE_WIDGETS && oldScreen != currentScreen)
    lv_indev_set_group(encoder_indev, group[currentScreen]);//Set group for encoder so that the encoder can control the objects in group
  currentScreen = handleScreen(currentScreen, focused_obj);
  updateVbatt();  // Update Battery Voltage measurement
  //changeHV();     // Change HV regulators voltage with rotary encoder
  //pbHV_Control(); // Turn HV regulators on/off with rotary pushbutton  
  raw_ADC_data = updateVdiff(raw_ADC_data, sampleAverages);  // Update Differential voltage measurement. 
  timings(&sysOpt);
  
}

//  Following Functions are just for intial testing purposes - To be removed

void changeHV(void){
  if (result || result_slow) {
    
    if (result == DIR_CW || result_slow == DIR_CW) {
      if(++Counter > 255) Counter = 0;
      vampSet += 0.5;
      if(vampSet > 15.0) 
        vampSet = 15.0;
      iset += 50;  
      if (iset > 0xFFC)
        iset = 0xFFC;
      vdiffNull += 50; 
      if(vdiffNull > 4000) vdiffNull = 4000;
    }
    else {
      if (--Counter < 0) Counter = 255;
      vampSet -= 0.5;
      if(vampSet < 4.0) 
        vampSet = 4.0;
      iset -= 51;  
      if (iset < 50)
        iset = 50;
      vdiffNull -= 50;   
      if(vdiffNull < 50) vdiffNull = 50;
    }
    //ledcWrite(0, Counter);
    dacVals[1] = iset;
    dac4ch.writeAllDACsFast(dacVals);
    dac4ch.updateDACs();
    /*
    vcalc = dac4ch.vampp2vdac(vampSet);
    dacVals[2] = dac4ch.float2dac(vcalc);
    vcalc = dac4ch.vampn2vdac(-vampSet);
    dacVals[3] = dac4ch.float2dac(vcalc); 
    dac4ch.writeAllDACsFast(dacVals);
    dac4ch.updateDACs();
    */   
  
    // dac1ch.writeDACFast(vdiffNull);
  }
}

void updateVbatt(void){
  if(++ctr >= 25) {
    ctr = 0;
    //if (digitalRead(EN_HPWR))
    //  digitalWrite(EN_HPWR, LOW);
    //else 
    //  digitalWrite(EN_HPWR, HIGH);
    dispVolt += 0.01;
    ++updDacsCnt;
    //esp_adc_cal_characteristics_t adc_chars;


    // Get the internal calibration value of the chip
    //esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    analogSetAttenuation(adc_attenuation_t::ADC_11db);
    //uint32_t raw = analogRead(PIN_BAT_VOLT);
    //uint32_t v1 = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2; //The partial pressure is one-half
    //batValue = v1 / 1000.0;
    float batValue = analogReadMilliVolts(PIN_BAT_VOLT)/1000.0;
    voltReadings.vbatt = batValue;

    lv_textarea_set_text(ui_VoltgeRT, String(batValue).c_str());
    //if(++battpc > 100) battpc = 20;
    bMon.getBatterySoC(battChargeLeft);
    battpc = uint8_t(battChargeLeft);
    lv_bar_set_value(ui_BattInf, battpc, LV_ANIM_OFF);
    //battpc = Counter;
    sprintf(txtd, "%d%%", battpc);
    lv_label_set_text(ui_BattLevLbl, txtd);
    bMon.getBatteryVoltage(battVoltage);
    sprintf(txtd, "%.4fV", battVoltage);
    lv_label_set_text(ui_VBattVal, txtd);
    sampleAverages = lv_dropdown_get_selected(ui_SampleAvgDn);
    sampleAverages = uint16_t(pow(2, sampleAverages));

    uint16_t activeTab = lv_tabview_get_tab_act(ui_MainPageTabs); // Get active tab page
    //sprintf(txtd, "Rst: %d", reset_reason);  // Just for debugging
    //lv_label_set_text(ui_SysInfoLbl, txtd);
  }
}

void pbHV_Control(void){
  if(switchPressed){
    switchPressed = false;
    if(digitalRead(EN_HPWR) == LOW) {
      enableHV_Regs(VAMP_DEFAULT);
    }
    else {
      digitalWrite(EN_HPWR, LOW);
    }
  }
}