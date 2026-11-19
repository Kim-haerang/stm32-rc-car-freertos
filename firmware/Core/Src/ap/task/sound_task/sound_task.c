#include "sound_task.h"
#include "../../message.h"
#include "../../../driver/buzzer/buzzer.h"
#include "tim.h"

#define C5 524U
#define E5 660U
#define G5 784U
#define C6 1048U
#define E6 1320U
#define F6 1396U
#define G6 1568U

typedef enum {
    SOUND_RC,
    SOUND_AUTO,
    SOUND_SEMI,
    SOUND_BEEP,
    SOUND_START
} sound_command_t;

typedef struct {
    uint16_t frequency_hz;
    uint16_t duration_ms;
} sound_note_t;

typedef struct {
    const sound_note_t *notes;
    uint8_t count;
} sound_song_t;

static const sound_note_t rc_notes[] = {
    {C5, 50}, {E5, 50}, {G5, 80}, {0, 500}
};
static const sound_note_t auto_notes[] = {
    {G5, 50}, {C6, 80}, {G5, 50}, {0, 500}
};
static const sound_note_t semi_notes[] = {
    {E5, 40}, {0, 30}, {E5, 40}, {G5, 70}, {0, 500}
};
static const sound_note_t beep_notes[] = {
    {F6, 10}, {G6, 10}, {E6, 10}, {0, 30}
};
static const sound_note_t start_notes[] = {
    {C5, 50}, {E5, 50}, {G5, 50}, {C5, 50}, {0, 150}
};

#define SONG(notes_array) {notes_array, sizeof(notes_array) / sizeof(notes_array[0])}
static const sound_song_t rc_song = SONG(rc_notes);
static const sound_song_t auto_song = SONG(auto_notes);
static const sound_song_t semi_song = SONG(semi_notes);
static const sound_song_t beep_song = SONG(beep_notes);
static const sound_song_t start_song = SONG(start_notes);

static sound_command_t current_mode = SOUND_RC;
static const sound_song_t *current_song;
static uint8_t note_index;
static uint32_t note_started_ms;

static const sound_song_t *mode_song(void)
{
    if (current_mode == SOUND_AUTO) {
        return &auto_song;
    }
    if (current_mode == SOUND_SEMI) {
        return &semi_song;
    }
    return &rc_song;
}

static void play_current_note(void)
{
    uint16_t frequency = current_song->notes[note_index].frequency_hz;
    if (frequency == 0) {
        buzzer_off();
    } else {
        buzzer_on(frequency);
    }
    note_started_ms = HAL_GetTick();
}

static void select_song(const sound_song_t *song)
{
    current_song = song;
    note_index = 0;
    play_current_note();
}

static void request_sound(sound_command_t command, bool replace_pending)
{
    if (soundQueHandle == NULL) {
        return;
    }
    if (replace_pending) {
        xQueueReset(soundQueHandle);
    }
    osMessagePut(soundQueHandle, command, 0);
}

void sound_task_init(void)
{
    buzzer_init(&htim4, TIM_CHANNEL_2);
    current_mode = SOUND_RC;
    select_song(&start_song);
}

void sound_task(void)
{
    osEvent event;
    while ((event = osMessageGet(soundQueHandle, 0)).status == osEventMessage) {
        sound_command_t command = (sound_command_t)event.value.v;
        if (command == SOUND_RC || command == SOUND_AUTO || command == SOUND_SEMI) {
            current_mode = command;
            select_song(mode_song());
        } else if (command == SOUND_BEEP) {
            select_song(&beep_song);
        } else if (command == SOUND_START) {
            select_song(&start_song);
        }
    }

    if ((uint32_t)(HAL_GetTick() - note_started_ms) >=
        current_song->notes[note_index].duration_ms) {
        note_index++;
        if (note_index >= current_song->count) {
            current_song = mode_song();
            note_index = 0;
        }
        play_current_note();
    }
}

void sound_beepbeep(void)
{
    request_sound(SOUND_BEEP, false);
}

void sound_start(void)
{
    request_sound(SOUND_START, false);
}

void sound_rc(void)
{
    request_sound(SOUND_RC, true);
}

void sound_auto(void)
{
    request_sound(SOUND_AUTO, true);
}

void sound_semi(void)
{
    request_sound(SOUND_SEMI, true);
}
