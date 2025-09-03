/*
 * ESP32-S3 Simple Tune Player for PlatformIO/Arduino using LEDC
 *
 * This program demonstrates a background tune player for the ESP32-S3 using the LEDC peripheral
 * to generate square-wave tones.
 * A dedicated FreeRTOS task manages a queue of tunes, allowing the main application to be non-blocking.
 * The tune player task takes control of the output pin only while playing and detaches it when done,
 * allowing the pin to be shared with other functions (like a buzzer).
 *
 * New functions have been added to gracefully stop and start the tune player,
 * which is necessary when entering a low-power mode or changing the core clock speed.
 */


/*
#include <Arduino.h>
#include "TuneManager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
//#include <FileFun.h>

static const char *TAG = "TUNE_PLAYER";

// The LEDC channel to use for the tune player. Should be different from other used channels (e.g., backlight, buzzer).
#define TUNE_LEDC_CHANNEL 2
// The GPIO pin to output the tone on.
#define TONE_PIN GPIO_NUM_11

extern void serialPrintDebug(const char * message, ...);

// Queue handle for sending tunes to the background task.
static QueueHandle_t tune_queue;
// Task handle to be able to stop and restart the tune player task.
static TaskHandle_t tune_player_task_handle = NULL;

// LEDC initialization function for tone generation.
static esp_err_t ledc_init_tone_mode() {
    // Prepare and then apply the LEDC PWM timer configuration
    // 5kHz frequency, 8-bit resolution
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_2, // Use Timer 2 to avoid conflict with backlight (0) and buzzer (1)
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .gpio_num = TONE_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = (ledc_channel_t)TUNE_LEDC_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_2, // Use Timer 2
        .duty = 0, // Set duty to 0%
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    return ESP_OK;
}

// Function to play a square wave for a given frequency and duration.
static void play_note(uint16_t frequency, uint16_t duration_ms) {
    ledcWriteTone(TUNE_LEDC_CHANNEL, frequency);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
}

// The background FreeRTOS task that plays tunes.
void tune_player_task(void *pvParameter) {
    serialPrintDebug("Tune player task started.");
    
    // The task will wait on the queue indefinitely.
    const tune_note_t *current_tune;
    while (xQueueReceive(tune_queue, &current_tune, portMAX_DELAY)) {
        serialPrintDebug("Playing a new tune on GPIO %d...", TONE_PIN);
        // Attach the LEDC channel to the GPIO pin to take control.
        ledcAttachPin(TONE_PIN, TUNE_LEDC_CHANNEL);

        // Iterate through the notes of the received tune.
        // The loop terminates only when a note with both frequency and duration of 0 is found.
        for (int i = 0; !(current_tune[i].frequency == 0 && current_tune[i].duration == 0); i++) {
            play_note(current_tune[i].frequency, current_tune[i].duration);
        }

        // Stop the tone and detach from the pin to release it for other uses (e.g., buzzer).
        ledcWriteTone(TUNE_LEDC_CHANNEL, 0);
        ledcDetachPin(TONE_PIN);

        serialPrintDebug("Tune finished.");
    }
    
    vTaskDelete(NULL); // This line is not expected to be reached.
}

// Function to send a tune to the player.
void play_tune(const tune_note_t *tune) {
    if (xQueueSend(tune_queue, &tune, portMAX_DELAY) != pdPASS) {
        serialPrintDebug("Failed to send tune to queue.");
    }
}

// Function to stop the tune player task.
void stop_tune_player() {
    serialPrintDebug("Stopping tune player...");
    if (tune_player_task_handle != NULL) {
        vTaskDelete(tune_player_task_handle);
        tune_player_task_handle = NULL;
    }
    // Ensure the pin is released and the tone is off, regardless of task state.
    // This prevents a note from getting "stuck" on if the task is deleted mid-play.
    ledcWriteTone(TUNE_LEDC_CHANNEL, 0);
    ledcDetachPin(TONE_PIN);
}

// Function to start the tune player task and initialize the LEDC peripheral.
void start_tune_player() {
    serialPrintDebug("Starting tune player...");
    // 1. Initialize the LEDC peripheral.
    ledc_init_tone_mode();
    
    // 2. Create the FreeRTOS queue for tunes if it doesn't exist.
    if (tune_queue == NULL) {
        tune_queue = xQueueCreate(10, sizeof(tune_note_t *));
        if (tune_queue == NULL) {
            serialPrintDebug("Failed to create tune queue.");
            return;
        }
    }
    
    // 3. Create the background tune player task.
    // The task's stack is allocated from internal RAM by default.
    xTaskCreatePinnedToCore(tune_player_task, "tune_player", 2048, NULL, 5, &tune_player_task_handle, 1);
}

*/
/*
 * ESP32-S3 Simple Tune Player for PlatformIO/Arduino using an external ticker.
 *
 * This version is driven by an external periodic call to tune_manager_tick(),
 * avoiding the need for a dedicated FreeRTOS task or its own timer.
 * A state machine plays notes sequentially.
 *
 * This approach is very memory-efficient.
 */
/*
 * ESP32-S3 Simple Tune Player for PlatformIO/Arduino using an external ticker.
 *
 * This version is driven by an external periodic call to tune_manager_tick(),
 * avoiding the need for a dedicated FreeRTOS task or its own timer.
 * A state machine plays notes sequentially.
 *
 * This approach is very memory-efficient.
 */

#include <Arduino.h>
#include "TuneManager.h"
#include "driver/ledc.h"
#include "esp_err.h"

// This external declaration allows us to use the debug print function from FileFun.cpp
extern void serialPrintDebug(const char * message, ...);

static const char *TAG = "TUNE_PLAYER_TICK";

// LEDC configuration
#define TUNE_LEDC_CHANNEL 2
#define TONE_PIN GPIO_NUM_11

// --- State machine variables ---
static const volatile tune_note_t *current_tune = NULL;
static volatile int current_note_index = 0;
static volatile int32_t note_duration_counter_ms = 0; // Use signed type for safe subtraction

// LEDC initialization
static void ledc_init_tone_mode() {
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_2, // Use Timer 2 to avoid conflict with backlight (0) and buzzer (1)
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .gpio_num = TONE_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = (ledc_channel_t)TUNE_LEDC_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_2, // Use Timer 2
        .duty = 0, // Set duty to 0%
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

/**
 * @brief Advances the tune player state machine. Call this from a periodic timer.
 * This function should be called from a timer interrupt (e.g., every 4ms).
 */
void tune_manager_tick(void) {
    if (current_tune == NULL) return;

    const uint32_t tick_period_ms = 4; // Corresponds to the 4ms Ticker in HwFun.cpp

    // Decrement the note duration counter
    if (note_duration_counter_ms > 0) {
        note_duration_counter_ms -= tick_period_ms;
    }

    // If the note has finished playing, move to the next one
    if (note_duration_counter_ms <= 0) {
        const volatile tune_note_t *note = &current_tune[++current_note_index];

        // Check for the end-of-tune marker
        if (note->frequency == 0 && note->duration == 0) {
            ledcWriteTone(TUNE_LEDC_CHANNEL, 0);
            ledcDetachPin(TONE_PIN);
            current_tune = NULL; // Mark as finished
            return;
        }

        // Play the new note (or rest)
        ledcWriteTone(TUNE_LEDC_CHANNEL, note->frequency);

        // Set the duration for the new note
        note_duration_counter_ms = note->duration;
    }
}

// Function to send a tune to the player.
void play_tune(const tune_note_t *tune) {
    if (tune == NULL) return;

    serialPrintDebug("Playing a new tune on GPIO %d...", TONE_PIN);

    // Initialize state machine for the new tune
    current_tune = tune;
    current_note_index = 0;
    note_duration_counter_ms = current_tune[0].duration;

    // Attach pin and play the first note
    ledcAttachPin(TONE_PIN, TUNE_LEDC_CHANNEL);
    ledcWriteTone(TUNE_LEDC_CHANNEL, current_tune[0].frequency);
}

// Function to stop the tune player.
void stop_tune_player() {
    serialPrintDebug("Stopping tune player...");
    current_tune = NULL;

    // Ensure the pin is released and the tone is off.
    ledcWriteTone(TUNE_LEDC_CHANNEL, 0);
    ledcDetachPin(TONE_PIN);
}

// Function to start the tune player system.
void start_tune_player() {
    serialPrintDebug("Starting tune player (Ticker version)...");
    // 1. Initialize the LEDC peripheral.
    ledc_init_tone_mode();
}

