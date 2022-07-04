/************************************************************
 *
 * Copyright (c) 2022 Lian Ferrand
 *
 * Permission is hereby granted, free of charge, to any
 * person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice
 * shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of I Wanna Slay the Dragon of Bangan
 *
 ************************************************************/

#ifndef _AUDIO_H
#define _AUDIO_H

#include "loveylib/types.h"
#include "loveylib/stream.h"

// Disable audio, for testing
// Will replace all audio handling functions with NOPs

// Make sure audio isn't disabled in release
#ifndef NDEBUG
#define DISABLE_AUDIO
#endif

struct audio_frame_t {
  i16 left, right;
};

// Screw you emacs
#define sound_e sound_e : ufast
enum sound_e {
  SND_SHOOT = 0,
  SND_JUMP, SND_DJUMP, SND_VINEJUMP,
  SND_DEATH,

  SND_NOSPELL,
  SND_JUMPSPELL,
  SND_SHOOTSPELL,
  SND_SPEEDSPELL,

  SND_GETSPELL,

  SND_SAVE,

  SND_BREAKBLOCK,

  SND_THUNDER,

  SND_MIKOO, SND_MIKOODEFEATED,

  SND_COUNT
};
#undef sound_e
typedef ufast sound_t;

extern const char * const G_SoundNames[SND_COUNT];

typedef void *sound_handle_t;

#ifndef DISABLE_AUDIO

// Loads all sounds into memory
void InitAudio();

// Frees all sounds from memory
void FreeAudio();

///////////////////////////////////
// Sound interface

// Loops BGM
// If filename is "", stops BGM
// If filename couldn't be found, stops BGM
// If filename is the same as the last call
// to this function, it is a NOP
void PlayBGM(const char *filename);

// Plays sound
sound_handle_t PlaySound(sound_t snd);

// Stops all sounds
void StopAllSounds();

// Stop sound type
void StopSound(sound_t snd);

// Stop specific sound
void StopSound(sound_handle_t snd);

// Update audio backend
void UpdateAudio();

#else //DISABLE_AUDIO

// Add 'a' to change prototype
static inline void InitAudio(int a = 0) {(void)a;}
static inline void FreeAudio(int a = 0) {(void)a;}
static inline void PlayBGM(const char*, int a = 0) {(void)a;}
static inline sound_handle_t PlaySound(sound_t, int a = 0) {(void)a; return 0;}
static inline void StopAllSounds(int a = 0) {(void)a;}
static inline void StopSound(sound_t, int a = 0) {(void)a;}
static inline void StopSound(sound_handle_t, int a = 0) {(void)a;}
static inline void UpdateAudio(int a = 0) {(void)a;}

#endif //DISABLE_AUDIO

////////////////////////////////////
// Functions for platform layer

// Open ADPCM audio file
// Returns number of samples in file
uptr OpenADPCM(const char *filename);

// Read frames from ADPCM audio file
// Loops if EOF reached
void ReadADPCM(audio_frame_t *out, uptr frames);

// Close ADPCM audio file
void CloseADPCM();

#endif //_AUDIO_H
