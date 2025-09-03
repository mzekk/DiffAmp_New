#pragma once

#include "TuneManager.h"

// --- Note Definitions ---
// Frequencies for musical notes from C4 to B7.
// Makes tunes more readable than using raw frequency numbers.
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_REST 0 // A frequency of 0 is a rest

// --- Tune Definitions ---

// A simple tune played on startup.
const tune_note_t startup_tune[] = {
    {NOTE_C6, 100},
    {NOTE_F6, 100},
    {NOTE_REST, 0}     // End of tune marker
};

// A tune to indicate an error or failure.
const tune_note_t failure_tune[] = {
    {NOTE_G5, 200},
    {NOTE_C5, 400},
    {NOTE_REST, 0}
};

// A tune to indicate success.
const tune_note_t success_tune[] = {
    {NOTE_C6, 100},
    {NOTE_E6, 100},
    {NOTE_G6, 100},
    {NOTE_C7, 200},
    {NOTE_REST, 0}
};

// A more recognizable tune: "Jingle Bells"
// This version includes short pauses (NOTE_REST) between notes to create a more distinct and rhythmic melody.
const tune_note_t jingle_bells_tune[] = {
    // Jingle bells, jingle bells, jingle all the way...
    {NOTE_E5, 150}, {NOTE_REST, 25}, {NOTE_E5, 150}, {NOTE_REST, 25}, {NOTE_E5, 300}, {NOTE_REST, 50},
    {NOTE_E5, 150}, {NOTE_REST, 25}, {NOTE_E5, 150}, {NOTE_REST, 25}, {NOTE_E5, 300}, {NOTE_REST, 50},
    {NOTE_E5, 150}, {NOTE_REST, 25}, {NOTE_G5, 150}, {NOTE_REST, 25}, {NOTE_C5, 250}, {NOTE_REST, 25}, {NOTE_D5, 100}, {NOTE_REST, 25}, {NOTE_E5, 400}, {NOTE_REST, 100},

    // Oh what fun it is to ride in a one-horse open sleigh, hey!
    {NOTE_F5, 150}, {NOTE_REST, 25}, {NOTE_F5, 150}, {NOTE_REST, 25}, {NOTE_F5, 150}, {NOTE_REST, 25}, {NOTE_F5, 150}, {NOTE_REST, 25},
    {NOTE_F5, 150}, {NOTE_REST, 25}, {NOTE_E5, 150}, {NOTE_REST, 25}, {NOTE_E5, 150}, {NOTE_REST, 25}, {NOTE_E5, 150}, {NOTE_REST, 25},
    {NOTE_E5, 150}, {NOTE_REST, 25}, {NOTE_D5, 150}, {NOTE_REST, 25}, {NOTE_D5, 150}, {NOTE_REST, 25}, {NOTE_E5, 150}, {NOTE_REST, 25},
    {NOTE_D5, 300}, {NOTE_REST, 50}, {NOTE_G5, 300}, {NOTE_REST, 100},

    // Jingle bells, jingle bells, jingle all the way...
    {NOTE_E5, 150}, {NOTE_REST, 25}, {NOTE_E5, 150}, {NOTE_REST, 25}, {NOTE_E5, 300}, {NOTE_REST, 50},
    {NOTE_E5, 150}, {NOTE_REST, 25}, {NOTE_E5, 150}, {NOTE_REST, 25}, {NOTE_E5, 300}, {NOTE_REST, 50},
    {NOTE_E5, 150}, {NOTE_REST, 25}, {NOTE_G5, 150}, {NOTE_REST, 25}, {NOTE_C5, 250}, {NOTE_REST, 25}, {NOTE_D5, 100}, {NOTE_REST, 25}, {NOTE_E5, 400}, {NOTE_REST, 100},

    // Oh what fun it is to ride in a one-horse open sleigh
    {NOTE_F5, 150}, {NOTE_REST, 25}, {NOTE_F5, 150}, {NOTE_REST, 25}, {NOTE_F5, 150}, {NOTE_REST, 25}, {NOTE_F5, 150}, {NOTE_REST, 25},
    {NOTE_F5, 150}, {NOTE_REST, 25}, {NOTE_E5, 150}, {NOTE_REST, 25}, {NOTE_E5, 150}, {NOTE_REST, 25}, {NOTE_G5, 150}, {NOTE_REST, 25},
    {NOTE_G5, 150}, {NOTE_REST, 25}, {NOTE_F5, 150}, {NOTE_REST, 25}, {NOTE_D5, 150}, {NOTE_REST, 25}, {NOTE_C5, 400},

    {NOTE_REST, 0}     // End of tune marker
};
