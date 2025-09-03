#pragma once

#include <cstdint>

// Note structure to define a single note with frequency and duration.
// This must be accessible to any code that wants to define a tune.
struct tune_note_t {
    uint16_t frequency; // Frequency in Hz
    uint16_t duration;  // Duration in milliseconds
};

/**
 * @brief Starts the background tune player task and initializes the sound hardware.
 * This must be called once before any tunes can be played.
 */
void start_tune_player();

/**
 * @brief Stops the background tune player task and releases the sound hardware.
 * Useful for entering low-power modes.
 */
void stop_tune_player();

/**
 * @brief Sends a tune to be played by the background task.
 * The tune is an array of note_t, terminated by a note with frequency 0.
 * This function is non-blocking and returns immediately.
 * @param tune A pointer to the tune array. The array must be static or global.
 */
void play_tune(const tune_note_t *tune);

/**
 * @brief Advances the tune player state machine. Call this from a periodic timer.
 * This function should be called from a timer interrupt (e.g., every 1-10ms).
 */
void tune_manager_tick(void);
