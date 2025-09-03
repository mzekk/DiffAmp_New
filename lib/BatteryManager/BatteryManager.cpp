#include "BatteryManager.h"
#include "DiffAmp.h"
#include "pin_config.h"
#include "ui/ui.h"
#include "lv_conf.h"
#include "lvgl.h" /* https://github.com/lvgl/lvgl.git */
#include <MAX17040.h>
#include <PCA9554.h>

// External hardware objects needed by the manager
extern MAX17040 bMon;
extern PCA9554 ioExp;

BatteryManager::BatteryManager(sysOptions* options)
    : sysOpt(options),
      currentState(State::ACTIVE),
      soc(0.0f),
      sov(0.0f),
      chargerIsConnected(false),
      chargeCycleIsDisabled(false),
      chargerTestInProgress(false),
      chargeHysteresis(0.0f),
      lastChargerCheckTime(0),
      overchargeTestTime(0),
      sleepCycleCounter(0),
      maxSleepCycles(0),
      maxCharge(sysOpt->SleepMaxTime * MS_IN_ONE_HOUR / sysOpt->SleepTimeCycleMs) // Double check
{
    // Initial hardware read
    readHardwareState();
}

void BatteryManager::setState(State newState) {
    // Only perform actions if the state is actually changing to prevent redundant logic.
    if (currentState != newState) {
        serialPrintDebug("[BATT] State changing from %d to %d\n", static_cast<int>(currentState), static_cast<int>(newState));
        currentState = newState;

        // Centralized logic that runs *after* a state transition.
        // This is much cleaner than scattering this logic in multiple places.
        if (currentState == State::ACTIVE) {
            sleepCycleCounter = 0; // Reset sleep counter whenever we become active.
        }
    }
}

void BatteryManager::enterSleepState(bool init) {
    if(init){
        resetChargeLogic();
    }
    // Use the setter to ensure all state change logic is handled consistently.
    setState(State::SLEEPING);
}

void BatteryManager::enterActiveState(bool init) {
    if(init){
         resetChargeLogic();
    }
    // Use the setter to ensure all state change logic is handled consistently.
    setState(State::ACTIVE);
}

void BatteryManager::resetChargeLogic() {
    chargeHysteresis = 0.0f;
    chargeCycleIsDisabled = false; 
    overchargeTestTime = millis();
    sleepCycleCounter = 0;
    maxSleepCycles = (sysOpt->SleepMaxTime * MS_IN_ONE_HOUR) / sysOpt->SleepTimeCycleMs; 
    chargerTestInProgress = false;
    ioExp.portMode(ENABLE_CHARGER);
}

void BatteryManager::update(uint32_t currentTime) {
    readHardwareState();

    if (currentState == State::ACTIVE) {
        updateActiveMode(currentTime);
    } else if (currentState == State::SLEEPING) {
        updateSleepMode();
    }
    // Other states are terminal and don't require updates.
}

void BatteryManager::readHardwareState() {
    bMon.getBatterySoC(soc);
    bMon.getBatteryVoltage(sov);
    chargerIsConnected = (digitalRead(LDAC_CHRG) == LOW);
}

void BatteryManager::updateActiveMode(uint32_t currentTime) {
    static uint32_t battStateMaskTime = MASK_OFF, lastBattMaskTime = currentTime;
    float maxCharge = sysOpt->BattProtect ? SAFE_BATT_CHARGE : MAX_BATT_CHARGE;
    //serialPrintDebug("Active Mode - [BATT] SoC: %.1f%%, SoV: %.2fV\n", soc, sov);

    // --- Overcharge Protection Logic ---
    if(checkChargerPresence(maxCharge)){   
        overchargeTestTime = millis(); 
        if (chargerIsConnected && soc > maxCharge) {
            // Battery is full, disable charging to protect it.
            ioExp.portMode(DISABLE_CHARGER);
            chargeCycleIsDisabled = true;
            chargeHysteresis = BATTERY_RECHARGE_HYSTERESIS;
            overchargeTestTime = millis();
            serialPrintDebug("[BATT] Charge cycle disabled (overcharge protection).\n");
        }

    // --- Low Battery Shutdown Check ---
    // Only shut down if the charger is NOT connected.
        if (!chargerIsConnected && (soc < sysOpt->BattMinChargeLeft || sov < sysOpt->BattLowThreshold)) {            
            if(currentTime - lastBattMaskTime > battStateMaskTime){
                serialPrintDebug("[BATT] Battery too low! SoC: %.1f%%, SoV: %.2fV. Requesting to Shut down power.\n", soc, sov);
                setState(State::BATTERY_TOO_LOW);
                battStateMaskTime = MASK_15SEC;    
                lastBattMaskTime = currentTime;
            }
            else setState(State::ACTIVE);
        }
        else battStateMaskTime = MASK_OFF;           
    }
    updateChargerIcon();
}

void BatteryManager::updateSleepMode() {
    float maxCharge = sysOpt->BattProtect ? SAFE_BATT_CHARGE : MAX_BATT_CHARGE;
    // --- Sleep Timeout Check ---
    if (maxSleepCycles > 0 && --maxSleepCycles == 0) {
        setState(State::SLEEP_TIME_EXPIRED);
        return;
    }
    // --- Overcharge Protection Logic ---
    if(checkChargerPresence(maxCharge)){  
        if (chargerIsConnected) {
            // Reset sleep timer if charger is connected
            maxSleepCycles = (sysOpt->SleepMaxTime * MS_IN_ONE_HOUR) / sysOpt->SleepTimeCycleMs;

            if (soc > maxCharge) {
                ioExp.portMode(DISABLE_CHARGER);
                chargeCycleIsDisabled = true;
                chargeHysteresis = BATTERY_RECHARGE_HYSTERESIS;
            } 
            // Visual indicator (blinking LED)
            if(sleepCycleCounter > 1){
                ioExp.SetLeds(ALL_LEDS_OFF);
                sleepCycleCounter = 0;
            }
            else {                
                ioExp.SetLeds(DISP_PROBE_POLARITY_POS);
            }
            serialPrintDebug("Sleep Mode - [sleepCounter] %d \n", sleepCycleCounter);
        } 
    }
    // Charger is NOT connected            
    // Only check battery level periodically to save power
    if (++sleepCycleCounter >= sysOpt->SleepNumCyclesToMeas && !chargerIsConnected) {
        ioExp.SetLeds((chargeCycleIsDisabled) ? DISP_PROBE_POLARITY_POS : DISP_PROBE_POLARITY_NEG);
        sleepCycleCounter = 0;
        if (soc < sysOpt->BattMinChargeLeft || sov < sysOpt->BattLowThreshold) {
            setState(State::BATTERY_TOO_LOW);
        }
        ioExp.SetLeds(ALL_LEDS_OFF);
        if(chargeCycleIsDisabled){            
            chargerTestInProgress = true;
            ioExp.portMode(ENABLE_CHARGER); // Briefly enable to read pin
            overchargeTestTime = millis() - CHARGER_TEST_TIME_START_SLEEP - sysOpt->SleepTimeCycleMs;
        }
    }
    else overchargeTestTime = millis();        
}

bool BatteryManager::checkChargerPresence(float maxCharge) {
    if (chargeCycleIsDisabled) {
        // We previously disabled charging. Check if we should re-enable it.
        if (soc < (maxCharge - chargeHysteresis) && chargerTestInProgress == false) {
            // Battery has discharged enough, re-enable the charge cycle.
            ioExp.portMode(ENABLE_CHARGER);
            chargeCycleIsDisabled = false;
            chargeHysteresis = 0;
            serialPrintDebug("[BATT] Charge cycle re-enabled.\n");
            return true;
        }
        // Periodically check if the charger is still physically connected.    
        if (millis() - overchargeTestTime > CHARGER_TEST_TIME_START) {
             ioExp.portMode(ENABLE_CHARGER); // Briefly enable to read pin
             serialPrintDebug("[BATT] Enabling Charger for test. Time = %d\n", (millis() - overchargeTestTime));
             chargerTestInProgress = true;
        }   
        if (millis() - overchargeTestTime > CHARGER_TEST_TIME_END && chargerTestInProgress) {            
             if (digitalRead(LDAC_CHRG) == HIGH) { // Charger was removed
                chargeCycleIsDisabled = false;
             } else {
                ioExp.portMode(DISABLE_CHARGER); // Re-disable
             }
             serialPrintDebug("[BATT] Testing Charger Presence. State = %d Time = %d\n", int(chargeCycleIsDisabled), (millis() - overchargeTestTime));
             chargerTestInProgress = false;
             overchargeTestTime = millis();
        }       
        return false;
    } 
    else return true;
}


void BatteryManager::updateChargerIcon() {
    // Update the UI icon based on the current state
    static bool lastChargerState = false;
    static bool lastDisabledState = false;
    uint32_t now = millis();

    // Update icon only on state change or every ~1 second to reduce flicker
    if (chargerIsConnected != lastChargerState || chargeCycleIsDisabled != lastDisabledState || (now - lastChargerCheckTime > 1000)) {
        if (chargerIsConnected) {
            if (chargeCycleIsDisabled) {
                // Green icon: Connected and fully charged
                lv_obj_set_style_img_recolor(ui_ChargingImg, lv_color_hex(0x36F415), 0);
            } else {
                // Yellow icon: Connected and charging
                lv_obj_set_style_img_recolor(ui_ChargingImg, lv_color_hex(0xF7F039), 0);
            }
            lv_obj_set_style_img_recolor_opa(ui_ChargingImg, 255, 0);
        } else {
            // Dark icon: Not connected
            lv_obj_set_style_img_recolor(ui_ChargingImg, lv_color_hex(0x1E1E02), 0);
            lv_obj_set_style_img_recolor_opa(ui_ChargingImg, 255, 0);
        }
        lastChargerState = chargerIsConnected;
        lastDisabledState = chargeCycleIsDisabled;
        lastChargerCheckTime = now;
    }
}