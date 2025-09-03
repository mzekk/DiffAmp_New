#pragma once

#include "SysOptions.h"
#define CHARGER_TEST_TIME_START 9000        // Time in ms to start the charger presence test
#define CHARGER_TEST_TIME_START_SLEEP 8800  // Time in ms to start the charger presence test
#define CHARGER_TEST_TIME_END   9300        // Time in ms to sample the charger presence
#define MASK_15SEC              15000       // Time in ms to skip battery monitoring while in Active mode
#define MASK_OFF                0           // Battery is always monitored


class BatteryManager {
public:
    // A clear, readable enum for the battery system's state.
    enum class State {
        ACTIVE,
        SLEEPING,
        CHECK_CHARGER_PRESENCE,
        BATTERY_TOO_LOW,    // A critical state indicating shutdown is required.
        SLEEP_TIME_EXPIRED  // A terminal state after max sleep time.
    };

    /**
     * @brief Constructor for the BatteryManager.
     * @param options A pointer to the global system options struct.
     */
    BatteryManager(sysOptions* options);

    /**
     * @brief The main update loop for the battery manager. Call this regularly.
     */
    void update(uint32_t currentTime);

    /**
     * @brief Puts the battery manager into the SLEEPING state.
     */
    void enterSleepState(bool init);
    void enterActiveState(bool init);
    void setState(State newState);


    // --- Public Getters for Status ---
    State getState() const { return currentState; }
    float getSoC() const { return soc; }
    float getSoV() const { return sov; }
    bool isChargerConnected() const { return chargerIsConnected; }
    bool isChargeCycleComplete() const { return chargeCycleIsDisabled; }

private:
    // --- Private Methods for Internal Logic ---
    void readHardwareState();
    void updateActiveMode(uint32_t currentTime);
    void updateSleepMode();
    void resetChargeLogic();
    bool checkChargerPresence(float maxCharge);
    void updateChargerIcon();

    // --- Member Variables to Hold State ---
    sysOptions* sysOpt;
    State currentState;
    float maxCharge;

    // Hardware state
    float soc; // State of Charge (%)
    float sov; // State of Voltage (V)
    bool chargerIsConnected;
    bool chargerTestInProgress;


    // Logic state
    bool chargeCycleIsDisabled; // True if we've stopped charging due to overcharge.
    float chargeHysteresis;     // Hysteresis value to prevent charge cycling.
    uint32_t lastChargerCheckTime;
    uint32_t overchargeTestTime;

    // Sleep state
    uint32_t sleepCycleCounter;
    uint32_t maxSleepCycles;
};