#include "ui/ui.h"
#include "EncoderRead.h"
#include "DiffAmp.h"
#include "pin_config.h"
#include "SysOptions.h"

#define VDIFF_TAB   0
#define OHM_TAB     1
#define DIODE_TAB   2
#define SMU_TAB     3
#define OPTIONS_TAB 4
#define MORE_VOLTS_SCR 5
#define GRAPH_SCR 6
#define MORE_OPTIONS_SCR 7
#define WIFI_SCR 8
#define MSG_SCR 9


extern lv_style_t focus_style;
extern lv_indev_t *encoder_indev;
extern lv_group_t *group[8];
extern uint8_t currentScreen;
extern lv_group_t *group_tabs;
extern EncoderRead encoder;
extern struct sysOptions sysOpt;
extern vi_limits ohm_vi_Limits;
extern vi_limits diode_vi_Limits;
extern void waitKeyRelease(void); 

enum FocusMode { MODE_WIDGETS, MODE_TABS };
extern FocusMode current_mode;
extern bool switchPressed, switchLongPressed;
extern volatile bool homePressed;
extern bool uiActions;
extern bool backLightDimmed;

extern vi_limits initUI_OhmData(sysOptions* sysOpt, uint16_t range);
extern vi_limits initUI_DiodeData(sysOptions* sysOpt, uint16_t range);
extern void screen_touch_event_cb(lv_event_t * e);

void checkBoxHandler(lv_obj_t *focus_obj, uint8_t screen);
bool wait_and_show_standby_screen(uint32_t timeout, uint8_t mode); 

volatile uint16_t changeTabDelay = 0;

int active_tab = 0;

// *** VoltPage ***
// ui_VinDividerBox     // 25 or 250 divider 
// ui_voltRangeDn       // Voltage FS range
// *** SMU Page ***
// ui_VoltLimitSpin     // Change Compliance Voltage
// ui_CurrLimitSpin
// ui_SMUOnOffBtn

// *** MainTabs ***
// ui_VoltPage
// ui_OhmPage
// ui_DiodePage
// ui_SMUPage
// ui_Options

void init_focus_style() {
    lv_style_init(&focus_style);
    lv_style_set_border_color(&focus_style, lv_palette_main(LV_PALETTE_RED));  // change color
    lv_style_set_border_width(&focus_style, 2);  // make thicker
    lv_style_set_border_side(&focus_style, LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_RIGHT); // all sides

    // Optional: Make it rounded
    //lv_style_set_radius(&focus_style, 5);
}

void apply_focus_style(lv_obj_t *obj) {
    lv_obj_add_style(obj, &focus_style, LV_PART_MAIN | LV_STATE_FOCUSED);
}

void add_widget_to_group_with_focus_style(lv_group_t *group, lv_obj_t *obj) {
  lv_group_add_obj(group, obj);
  lv_obj_add_style(obj, &focus_style, LV_PART_MAIN | LV_STATE_FOCUSED);
}

void ui_create_groups(void){
    //Set Volt Page group
    group[0] = lv_group_create();   
    //lv_group_set_default(group[0]);
    //add_widget_to_group_with_focus_style(group[0], ui_VinDividerBox); //instead of lv_group_add_obj to apply style for encoder operation.
    add_widget_to_group_with_focus_style(group[0], ui_VoltRangeDn);
    add_widget_to_group_with_focus_style(group[0], ui_HoldBtn);
    add_widget_to_group_with_focus_style(group[0], ui_PeakBtn);
    add_widget_to_group_with_focus_style(group[0], ui_RelBtn);
    //Set Ohm Page group
    group[1] = lv_group_create();   
    add_widget_to_group_with_focus_style(group[1], ui_OhmSelDn);
    add_widget_to_group_with_focus_style(group[1], ui_OhmModeCb);
    add_widget_to_group_with_focus_style(group[1], ui_OhmTrigBtn);
    add_widget_to_group_with_focus_style(group[1], ui_OhmBuzzBtn);
    //Set Diode Page group
    group[2] = lv_group_create();   
    add_widget_to_group_with_focus_style(group[2], ui_DiodeVfSelDn);
    add_widget_to_group_with_focus_style(group[2], ui_DiodeMeasModeCb);
    add_widget_to_group_with_focus_style(group[2], ui_DiodeTrigBtn);
    add_widget_to_group_with_focus_style(group[2], ui_DiodeBuzzBtn);
     //Set SMU Page group
    group[3] = lv_group_create();  
    add_widget_to_group_with_focus_style(group[3], ui_VoltLimitSpin);
    add_widget_to_group_with_focus_style(group[3], ui_CurrLimitSpin);
    add_widget_to_group_with_focus_style(group[3], ui_SMUOnOffBtn);
    //Set Options Page group
    group[4] = lv_group_create();  
    add_widget_to_group_with_focus_style(group[4], ui_StandbyOptDn); 
    add_widget_to_group_with_focus_style(group[4], ui_FlipDispCntrl);  
    add_widget_to_group_with_focus_style(group[4], ui_MaxBrightnessCtrl);  
    add_widget_to_group_with_focus_style(group[4], ui_MinBrightnessCtrl);  
    add_widget_to_group_with_focus_style(group[4], ui_BacklightToutCtrl);  
    add_widget_to_group_with_focus_style(group[4], ui_MoreOptionsBtn);  // Now is ui_MoreOptionsBtn button, change in next ui update
    
    //Set Volt More Screen group
    group[5] = lv_group_create(); 
    add_widget_to_group_with_focus_style(group[5], ui_VdiffGainSelectDn); 
    add_widget_to_group_with_focus_style(group[5], ui_VprobeGainSelectDn); 
    add_widget_to_group_with_focus_style(group[5], ui_SampleAvgDn); 
    add_widget_to_group_with_focus_style(group[5], ui_SampleRateDn); 
    add_widget_to_group_with_focus_style(group[5], ui_RetVoltBtn); 
    //Set Graph Screen Page group
    group[6] = lv_group_create(); 
    add_widget_to_group_with_focus_style(group[6], ui_VdiffCb); 
    add_widget_to_group_with_focus_style(group[6], ui_VprobePCb); 
    add_widget_to_group_with_focus_style(group[6], ui_VprobeNCb); 
    add_widget_to_group_with_focus_style(group[6], ui_VdriveCb); 
    add_widget_to_group_with_focus_style(group[6], ui_IsenseCb); 
    add_widget_to_group_with_focus_style(group[6], ui_GraphRetBtn); 
    //Set More Options Page group
    group[7] = lv_group_create(); 
    add_widget_to_group_with_focus_style(group[7], ui_TimeZonesDn); 
    add_widget_to_group_with_focus_style(group[7], ui_DaylightCorrCb); 
    add_widget_to_group_with_focus_style(group[7], ui_KeepOnWithChargerCb); 
    add_widget_to_group_with_focus_style(group[7], ui_BatteryProtectionCb); 
    add_widget_to_group_with_focus_style(group[7], ui_CalListDn); 
    add_widget_to_group_with_focus_style(group[7], ui_ExecCalBtn); 
    add_widget_to_group_with_focus_style(group[7], ui_MaxSleepTimeCtrl); 
    add_widget_to_group_with_focus_style(group[7], ui_WirelessSetupBtn); 
    add_widget_to_group_with_focus_style(group[7], ui_MoreOptionsRetBtn); 
    //Set WiFi Page group
    group[8] = lv_group_create();  
    add_widget_to_group_with_focus_style(group[8], ui_SSIDListDn); 
    add_widget_to_group_with_focus_style(group[8], ui_PasswordArea);
    add_widget_to_group_with_focus_style(group[8], ui_WiFiScanBtn);
    add_widget_to_group_with_focus_style(group[8], ui_WiFiReturnBtn); 

    //Set Message Page group
    group[9] = lv_group_create(); 
    add_widget_to_group_with_focus_style(group[9], ui_MsgYesBtn); 
    add_widget_to_group_with_focus_style(group[9], ui_MsgNoBtn); 
    
    //Set MainTabs group
    group_tabs = lv_group_create();
    lv_group_add_obj(group_tabs, lv_tabview_get_tab_btns(ui_MainPageTabs));
    //add_widget_to_group_with_focus_style(group_tabs, lv_tabview_get_tab_btns(ui_MainPageTabs));


    // Set initial group
    lv_indev_set_group(encoder_indev, group[0]); 
    lv_group_set_default(group[0]);
}

void switch_to_tab_group(int tab) {
  active_tab = tab;
  lv_indev_set_group(encoder_indev, group[tab]);
  current_mode = MODE_WIDGETS;
  lv_tabview_set_act(ui_MainPageTabs, tab, LV_ANIM_ON);
}

void switch_to_tab_buttons() {
  lv_indev_set_group(encoder_indev, group_tabs);
  current_mode = MODE_TABS;
}


uint8_t get_current_screen(uint8_t currentScrID, lv_obj_t* currentScrLV){
    if (currentScrLV ==ui_Main) {
        uint16_t activeTab = lv_tabview_get_tab_act(ui_MainPageTabs); // Get active tab page
        switch (activeTab)
        {
            case VDIFF_TAB:
            return VDIFF_TAB;
            case OHM_TAB:
            return OHM_TAB;
            case DIODE_TAB:
            return DIODE_TAB;
            case SMU_TAB:
            return SMU_TAB;
            case OPTIONS_TAB:
            return OPTIONS_TAB;
            default:
            return VDIFF_TAB;
        }
        return 0;
    }
    else if (currentScrLV==ui_Volt_More) return MORE_VOLTS_SCR;
    else if (currentScrLV==ui_Graph) return GRAPH_SCR;
    else if (currentScrLV==ui_MoreOptions) return MORE_OPTIONS_SCR;
    else if (currentScrLV==ui_WiFi) return WIFI_SCR;
    else if (currentScrLV==ui_MessagesScr) return MSG_SCR;
    else return currentScrID;
  }

uint8_t handleScreen(uint8_t screen, lv_obj_t *focused_obj){
    //bool longPress = encoder.encBtnLongPress();
    /*
    if (switchLongPressed && current_mode == MODE_WIDGETS) {
          //lv_group_focus_obj(NULL);        // Clear focus
          //lv_group_focus_obj(focused_obj); // Refocus spinbox in non-edit mode
          switch_to_tab_buttons();
          switchLongPressed = false;
          current_mode = MODE_TABS;
          
        }
    else if (switchLongPressed && current_mode == MODE_TABS) {
          //uint16_t activeTab = lv_tabview_get_tab_act(ui_MainPageTabs); // Get active tab page
          //lv_group_focus_obj(NULL);        // Clear focus
          //lv_group_focus_obj(focused_obj); // Refocus spinbox in non-edit mode
          switch_to_tab_group(screen);
          switchLongPressed = false;
          current_mode = MODE_WIDGETS;
    }
    */
    if(homePressed && changeTabDelay == 0 && screen <= OPTIONS_TAB && backLightDimmed == false){
      
      homePressed = false;
      if(screen == OPTIONS_TAB)
        screen = VDIFF_TAB;
      else screen++;  
      lv_tabview_set_act(ui_MainPageTabs, screen, LV_ANIM_OFF); // Change tab sequentially
      lv_indev_set_group(encoder_indev, group[screen]);//Set group for encoder so that the encoder can control the objects in group
      changeTabDelay = TAB_DELAY;
    }
    else if(homePressed && backLightDimmed == true) {
      uiActions = true;
      homePressed = false;
      changeTabDelay = TAB_DELAY;
    }

    if(changeTabDelay > 0)
      homePressed = false;
;

    switch(screen) {
      case VDIFF_TAB: 
        
       if (switchPressed) {//Handle the objects on the screen when the rotary encoder button is pressed.
          /*
          if(focused_obj == lv_tabview_get_tab_btns(ui_MainPageTabs)){               
            uint16_t sel = lv_btnmatrix_get_selected_btn(focused_obj);
            lv_tabview_set_act(ui_MainPageTabs, sel, LV_ANIM_ON);    
            lv_group_focus_next(group[0]);
            Serial.println("A Button was pressed on main screen ");
          } 
          else if (focused_obj == ui_VinDividerBox ){
                lv_event_send(focused_obj, LV_EVENT_PRESSED, NULL);//Send the button press event for processing.
                lv_obj_clear_state(focused_obj, LV_STATE_PRESSED); //Set effect when click button
                lv_obj_add_state(focused_obj, LV_STATE_DEFAULT); 
          } 
          */   
        }
      break;
      case OHM_TAB: 
        checkBoxHandler(focused_obj, screen);
        if(lv_obj_has_state(ui_OhmBuzzBtn, LV_STATE_CHECKED))
		      sysOpt.Ohm_Buzz= true;
	      else
		      sysOpt.Ohm_Buzz = false;
      break;
      case DIODE_TAB: 
        checkBoxHandler(focused_obj, screen); 
        if(lv_obj_has_state(ui_DiodeBuzzBtn, LV_STATE_CHECKED))
		      sysOpt.Diode_Buzz= true;
	      else
		      sysOpt.Diode_Buzz = false;
      break;
      case SMU_TAB: 
  
        if((focused_obj == ui_VoltLimitSpin | focused_obj == ui_CurrLimitSpin) && switchLongPressed){
          lv_group_focus_obj(NULL);        // Clear focus
          lv_group_focus_obj(focused_obj); // Refocus spinbox in non-edit mode
          switchLongPressed = false;
        }           
      break;
      case OPTIONS_TAB: 
        checkBoxHandler(focused_obj, screen);
       //if (encoder.encBtn()) {//Handle the objects on the screen when the rotary encoder button is pressed.
       //   if(focused_obj == ui_VoltPage 
       //       || focused_obj == ui_OhmPage || focused_obj == ui_DiodePage 
       //       || focused_obj == ui_SMUPage || focused_obj == ui_Options){
       //         Serial.println("A Button was pressed on main screen ");
       //   } 
       // }
      break;
      case MORE_VOLTS_SCR: 
      break;
      
      case GRAPH_SCR: 
        checkBoxHandler(focused_obj, screen);
      break;

      case MORE_OPTIONS_SCR: 
        checkBoxHandler(focused_obj, screen);
      break;

      case WIFI_SCR: 
        checkBoxHandler(focused_obj, screen);
      break;

      case MSG_SCR: 
      break;
    }
    sysOpt.LastScreen = screen;
    if(switchPressed)
      switchPressed = false;
    return screen;
}

void initUIdata(sysOptions* sysOpt) {
  char txtd[25];
  lv_dropdown_set_selected(ui_SampleAvgDn, sysOpt->ADC_Averages);
  lv_dropdown_set_selected(ui_SampleRateDn, sysOpt->ADC_Sample_Rate);
  lv_dropdown_set_selected(ui_VdiffGainSelectDn, sysOpt->ADC_VdiffGain);
  lv_dropdown_set_selected(ui_VprobeGainSelectDn, sysOpt->ADC_VnPGain);
  
  sprintf(txtd, "Brightness: [%d%%]", sysOpt->BacklightBrightness);
	lv_label_set_text(ui_MaxBrightnessLbl, txtd);
  lv_slider_set_value(ui_MaxBrightnessCtrl, sysOpt->BacklightBrightness, LV_ANIM_OFF);
  sprintf(txtd, "LP Brighthness: [%d%%]", sysOpt->BacklightLowBrght);
	lv_label_set_text(ui_MinBrightnessLbl, txtd);
  lv_slider_set_value(ui_MinBrightnessCtrl, sysOpt->BacklightLowBrght, LV_ANIM_OFF);
  sprintf(txtd, "BNL Timeout: [%d sec]", sysOpt->BacklightTout * 5);
	lv_label_set_text(ui_BacklightToutLbl, txtd);
  lv_slider_set_value(ui_BacklightToutCtrl, sysOpt->BacklightTout, LV_ANIM_OFF);
 
  lv_obj_add_state(ui_BatteryProtectionCb, sysOpt->BattProtect); // LV_STATE_CHECKED if true, LV_STATE_DEFAULT if false

  lv_obj_add_state(ui_DiodeBuzzBtn, sysOpt->Diode_Buzz); // LV_STATE_CHECKED if true, LV_STATE_DEFAULT if false
  lv_obj_add_state(ui_DiodeMeasModeCb, sysOpt->Diode_MeasMode); // LV_STATE_CHECKED if true, LV_STATE_DEFAULT if false
  lv_dropdown_set_selected(ui_DiodeVfSelDn, sysOpt->DiodeType);
  lv_obj_add_state(ui_OhmBuzzBtn, sysOpt->Ohm_Buzz); // LV_STATE_CHECKED if true, LV_STATE_DEFAULT if false
  lv_obj_add_state(ui_OhmModeCb, sysOpt->Ohm_MeasMode); // LV_STATE_CHECKED if true, LV_STATE_DEFAULT if false
  lv_dropdown_set_selected(ui_OhmSelDn, sysOpt->Ohm_Range);
  lv_obj_add_state(ui_FlipDispCntrl, sysOpt->Opt_FlipScreen); // LV_STATE_CHECKED if true, LV_STATE_DEFAULT if false
  lv_obj_add_state(ui_KeepOnWithChargerCb, sysOpt->SleepWithCharger); // LV_STATE_CHECKED if true, LV_STATE_DEFAULT if false

  if(sysOpt->Opt_FlipScreen) {
		lv_disp_set_rotation(lv_disp_get_default(), LV_DISP_ROT_180);
		sysOpt->Opt_FlipScreen = true;
	}
	else 
		lv_disp_set_rotation(lv_disp_get_default(), LV_DISP_ROT_NONE);
 
  lv_spinbox_set_value(ui_VoltLimitSpin, sysOpt->SMU_V_Lim*1000); 
  lv_spinbox_set_value(ui_CurrLimitSpin, sysOpt->SMU_I_Lim*1000); 
  lv_dropdown_set_selected(ui_StandbyOptDn, sysOpt->StandbyTout);
  lv_dropdown_set_selected(ui_TimeZonesDn, (sysOpt->TimeZone + 12));
  lv_obj_add_state(ui_DaylightCorrCb, sysOpt->TimeDaylightOffset); // LV_STATE_CHECKED if true, LV_STATE_DEFAULT if false
  lv_dropdown_set_selected(ui_VoltRangeDn, sysOpt->Volt_Range);
  ohm_vi_Limits = initUI_OhmData(sysOpt, sysOpt->Ohm_Range);
  diode_vi_Limits = initUI_DiodeData(sysOpt, sysOpt->DiodeType);

  sprintf(txtd, "Max Time in Sleep [%dHr]", sysOpt->BacklightTout * 5);
	lv_label_set_text(ui_BacklightToutLbl, txtd);
  lv_slider_set_value(ui_MaxSleepTimeCtrl, sysOpt->SleepMaxTime, LV_ANIM_OFF);
 
  sprintf(txtd, "Rev.[%d.%d.%d]", sysOpt->Revision&0x0F, (sysOpt->Revision>>4)&0x0F, (sysOpt->Revision>>8)&0x0F);
  lv_label_set_text(ui_SysInfoLbl, txtd);

}

vi_limits initUI_OhmData(sysOptions* sysOpt, uint16_t range){
  vi_limits lims;
  char sVlim[20], sIlim[20];
  switch(range){
    case R_AUTO:
      sprintf(sVlim, "VLIM = %.1f V", sysOpt->Ohm_V_10MR);     
      sprintf(sIlim, "ILIM = %3g uA", sysOpt->Ohm_I_10MR);
      lims.voltSet = sysOpt->Ohm_V_10MR;
      lims.currSet = sysOpt->Ohm_I_10MR;
    break;
    case R_1:
      sprintf(sVlim, "VLIM = %.1f V", sysOpt->Ohm_V_1R);
      sprintf(sIlim, "ILIM = %3g mA", sysOpt->Ohm_I_1R/1000);
      lims.voltSet = sysOpt->Ohm_V_1R;
      lims.currSet = sysOpt->Ohm_I_1R*1000;
    break;
    case R_1K:
      sprintf(sVlim, "VLIM = %.1f V", sysOpt->Ohm_V_1kR);
      sprintf(sIlim, "ILIM = %3g uA", sysOpt->Ohm_I_1kR);
      lims.voltSet = sysOpt->Ohm_V_1kR;
      lims.currSet = sysOpt->Ohm_I_1kR;
    break;
    case R_10K:
      sprintf(sVlim, "VLIM = %.1f V", sysOpt->Ohm_V_10kR);
      sprintf(sIlim, "ILIM = %3g uA", sysOpt->Ohm_I_10kR);
      lims.voltSet = sysOpt->Ohm_V_10kR;
      lims.currSet = sysOpt->Ohm_I_10kR;
    break;
    case R_100K:
      sprintf(sVlim, "VLIM = %.1f V", sysOpt->Ohm_V_100kR);
      sprintf(sIlim, "ILIM = %3g uA", sysOpt->Ohm_I_100kR);
      lims.voltSet = sysOpt->Ohm_V_100kR;
      lims.currSet = sysOpt->Ohm_I_100kR;
      break;
    case R_1M:
      sprintf(sVlim, "VLIM = %.1f V", sysOpt->Ohm_V_1MR);  
      sprintf(sIlim, "ILIM = %3g uA", sysOpt->Ohm_I_1MR); 
      lims.voltSet = sysOpt->Ohm_V_1MR;
      lims.currSet = sysOpt->Ohm_I_1MR; 
      break;
    case R_10M:
      sprintf(sVlim, "VLIM = %.1f V", sysOpt->Ohm_V_10MR);  
      sprintf(sIlim, "ILIM = %3g uA", sysOpt->Ohm_I_10MR);
      lims.voltSet = sysOpt->Ohm_V_10MR;
      lims.currSet = sysOpt->Ohm_I_10MR;  
      break; 
    case R_1M_HV:   
      sprintf(sVlim, "VLIM = %.1f V", RES_HV_VLIM); 
      sprintf(sIlim, "ILIM = %3g uA", sysOpt->Ohm_I_1MR);
      lims.voltSet = RES_HV_VLIM;
      lims.currSet = sysOpt->Ohm_I_1MR;
      break;
    case R_10M_HV:  
      sprintf(sVlim, "VLIM = %.1f V", RES_HV_VLIM); 
      sprintf(sIlim, "ILIM = %3g uA", sysOpt->Ohm_I_10MR);
      lims.voltSet = RES_HV_VLIM;
      lims.currSet = sysOpt->Ohm_I_10MR;
      break;  
    default:
      sprintf(sVlim, "VLIM = %.1f V", sysOpt->Ohm_V_1MR);     
      sprintf(sIlim, "VIIM = %3g uA", sysOpt->Ohm_I_1MR); 
      lims.voltSet = sysOpt->Ohm_V_1MR;
      lims.currSet = sysOpt->Ohm_I_1MR;
  }
  lv_label_set_text(ui_OhmVLimitVal, sVlim);
  lv_label_set_text(ui_OhmILimitVal, sIlim);
  return lims;

}

vi_limits initUI_DiodeData(sysOptions* sysOpt, uint16_t range){
  vi_limits lims;
  char sVlim[20], sIlim[20];
  switch(range){
    case LOWVF_DIODE:
      sprintf(sVlim, "VLIM = %2.1f V", sysOpt->Diode_V_LowVF);     
      sprintf(sIlim, "ILIM = %2.1f mA", sysOpt->Diode_I_LowVF/1000);
      lims.voltSet = sysOpt->Diode_V_LowVF;
      lims.currSet = sysOpt->Diode_I_LowVF;
    break;
    case ZENER_DIODE:
      sprintf(sVlim, "VLIM = %2.1f V", sysOpt->Diode_V_LED);     
      sprintf(sIlim, "ILIM = %2.1f mA", sysOpt->Diode_I_LED/1000);
      lims.voltSet = sysOpt->Diode_V_LED;
      lims.currSet = sysOpt->Diode_I_LED;
    break;
    case LED_DIODE:
      sprintf(sVlim, "VLIM = %2.1f V", sysOpt->Diode_V_Zener);     
      sprintf(sIlim, "ILIM = %2.1f mA", sysOpt->Diode_I_Zener/1000);
      lims.voltSet = sysOpt->Diode_V_Zener;
      lims.currSet = sysOpt->Diode_I_Zener;
    break;
    default:
      sprintf(sVlim, "VLIM = %2.1f V", sysOpt->Diode_V_LowVF);     
      sprintf(sIlim, "ILIM = %2.1f mA", sysOpt->Diode_I_LowVF/1000);
    break;
  }
  lv_label_set_text(ui_VdiodeLimitVal, sVlim);
  lv_label_set_text(ui_IdiodeLimitVal, sIlim);
  lims.voltSet = sysOpt->Diode_V_LowVF;
  lims.currSet = sysOpt->Diode_I_LowVF;
  return lims;  
}

void checkBoxHandler(lv_obj_t *focused_obj, uint8_t screen){
  //serialPrintDebug("Switch: %d\n", switchPressed);
  //if (switchPressed && (focused_obj == ui_DiodeMeasModeCb || focused_obj == ui_OhmModeCb || focused_obj == ui_FlipDispCntrl)) {//Handle the objects on the screen when the rotary encoder button is pressed.
  if (switchPressed){
    bool checked = lv_obj_has_state(focused_obj, LV_STATE_CHECKED);
    if (lv_obj_check_type(focused_obj, &lv_checkbox_class)){
      if (checked) {
        lv_obj_clear_state(focused_obj, LV_STATE_CHECKED);
        if(focused_obj == ui_DiodeMeasModeCb)
          lv_obj_add_state(ui_DiodeTrigBtn, LV_STATE_DISABLED);  
        if(focused_obj == ui_OhmModeCb)
          lv_obj_add_state(ui_OhmTrigBtn, LV_STATE_DISABLED);
        if(focused_obj == ui_FlipDispCntrl){
          lv_disp_set_rotation(lv_disp_get_default(), LV_DISP_ROT_NONE);
		      sysOpt.Opt_FlipScreen = true;
        } 
      } 
      else {
        lv_obj_add_state(focused_obj, LV_STATE_CHECKED);
        if(focused_obj == ui_DiodeMeasModeCb)
          lv_obj_clear_state(ui_DiodeTrigBtn, LV_STATE_DISABLED);
        if(focused_obj == ui_OhmModeCb)
          lv_obj_clear_state(ui_OhmTrigBtn, LV_STATE_DISABLED);  
        if(focused_obj == ui_FlipDispCntrl){
          lv_disp_set_rotation(lv_disp_get_default(), LV_DISP_ROT_180);
		      sysOpt.Opt_FlipScreen = false;
        }   
      }
      lv_group_focus_next(group[screen]);
      lv_group_focus_obj(NULL);        // Clear focus
      lv_group_focus_obj(focused_obj); // Refocus checkbox in non-edit mode
    }
  }
}

bool wait_and_show_standby_screen(uint32_t timeout_sec, uint8_t mode) {
    lv_obj_t* prev_screen = lv_scr_act();
    lv_indev_set_group(encoder_indev, group[MSG_SCR]);
    lv_obj_add_flag(ui_MsgNoBtn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_MsgYesBtn, LV_OBJ_FLAG_HIDDEN);

    lv_scr_load(ui_Messages);  // Show standby screen

    lv_obj_invalidate(ui_Messages);  // Ensure refresh

    if(mode == OFF_MSG_ID){
      serialPrintDebug("Turn Off requested. Wait Key Release\n");
      lv_textarea_set_text(ui_MessagesScr, "Turn Off requested.\n\n Release encoder to confirm");
      lv_timer_handler();  // LVGL refresh
      waitKeyRelease(); 
      serialPrintDebug("Key has been released.\n");
    }
     
    for (int i = timeout_sec; i > 0; --i) {
        char buf[80];
        if(mode == SLEEP_MSG_ID)
          sprintf(buf,"Standby in %d sec. \n\n Press Encoder to cancel", i);
        if(mode == LOWBATT_MSG_ID)  
          sprintf(buf,"[LOW BATTERY] - Turning Off in %d sec. \n\n Press Encoder to cancel", i);
        else
          sprintf(buf,"Turning Off in %d sec. \n\n Press Encoder to cancel", i);
        lv_textarea_set_text(ui_MessagesScr, buf);
        lv_timer_handler();  // LVGL refresh

        for(int j = 0; j < 10; j++) {
            delay(100);         // 100 ms step
            if (digitalRead(SWPB) == HIGH) {
                serialPrintDebug("Standby canceled by user.\n");
                lv_indev_set_group(encoder_indev, group[currentScreen]);
                lv_scr_load(prev_screen);  // Restore previous screen
                lv_timer_handler();  // LVGL refresh
                return false;
            }
        }       
    }
    serialPrintDebug("Waiting time expired.\n");
    // Sleep after countdown
    lv_indev_set_group(encoder_indev, group[currentScreen]);
    lv_scr_load(prev_screen);  // Restore previous screen
    lv_timer_handler();  // LVGL refresh
    return true;  // Proceed with sleep or OFF
}
