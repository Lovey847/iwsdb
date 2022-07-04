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

#include "loveylib/assert.h"
#include "audio.h"
#include "log.h"
#include "mem.h"
#include "str.h"
#include "loveylib/file.h"
#include "loveylib/endian.h"
#include "loveylib/timer.h"

#include <alsa/asoundlib.h>
#include <pthread.h>

#include <alloca.h>

extern timestamp_t g_timerFrequency; // From main.cpp

struct sound_channel {
  // Channel start frame, in sample buffer
  uptr startFrame;

  // If p == NULL, this sound channel is inactive
  audio_frame_t *p, *end;
  sound_t id;

  inline bptr playing() const {return (bptr)p;}
};

#define A_CHANNELS 2
#define A_SAMPLERATE 48000
#define A_SAMPLEFORMAT SND_PCM_FORMAT_S16_LE
#define A_BUFSIZE 1024
#define A_DEVICE "default"
#define A_ACCESS SND_PCM_ACCESS_RW_INTERLEAVED
#define A_SOUND_BUF_SIZE 6873504

static bfast a_bgm = false;
static char a_bgmName[64];
static char a_errbuf[256]; // Error string
static pthread_mutex_t a_m = PTHREAD_MUTEX_INITIALIZER;
static i16 *a_samples; // Sample buffer
static pthread_cond_t a_c = PTHREAD_COND_INITIALIZER;
static pthread_t a_tid; // Audio thread index
static volatile bfast a_quit = false; // Tells audio thread to exit infinite loop
static volatile timestamp_t a_bufPlayTime; // Time when buffer started being handled

static sound_channel s_sounds[SND_COUNT];
static audio_frame_t *s_soundBuf;

static constexpr const uptr SND_CHANNELS = 16;
static sound_channel s_channels[SND_CHANNELS];

// Initialize ALSA audio, in audio thread
static snd_pcm_t *a_init(void) {
  // Error checking macros
#define alsaCheck(_ret, ...)                                \
  if ((err = (_ret)) < 0) {                                 \
    sprintf(a_errbuf, __VA_ARGS__, err, snd_strerror(err)); \
    pthread_cond_signal(&a_c);                              \
    snd_pcm_close(handle);                                  \
    return NULL;                                            \
  }
#define alsaCheckNoClose(_ret, ...)                         \
  if ((err = (_ret)) < 0) {                                 \
    sprintf(a_errbuf, __VA_ARGS__, err, snd_strerror(err)); \
    pthread_cond_signal(&a_c);                              \
    return NULL;                                            \
  }
#define condCheck(_cond, ...)                   \
  if (_cond) {                                  \
    sprintf(a_errbuf, __VA_ARGS__);             \
    pthread_cond_signal(&a_c);                  \
    return NULL;                                \
  }

  snd_pcm_t *handle = NULL;
  snd_pcm_hw_params_t *hw;
  int err;

  // Initialize PCM device
  alsaCheckNoClose(snd_pcm_open(&handle, A_DEVICE, SND_PCM_STREAM_PLAYBACK, 0),
                   "Cannot open " A_DEVICE "! (%d, %s)\n");

  // Get default hardware configuration
  snd_pcm_hw_params_alloca(&hw);
  alsaCheck(snd_pcm_hw_params_any(handle, hw),
            "Cannot find any audio configurations! (%d, %s)\n");

  // Set channel count
  alsaCheck(snd_pcm_hw_params_set_channels(handle, hw, A_CHANNELS),
            "Cannot set number of channels to 2! (%d, %s)\n");

  // Set sample rate
  alsaCheck(snd_pcm_hw_params_set_rate(handle, hw, A_SAMPLERATE, 0),
            "Cannot set sample rate to %d! (%d, %s)\n", A_SAMPLERATE);

  // Set sample format
  alsaCheck(snd_pcm_hw_params_set_format(handle, hw, A_SAMPLEFORMAT),
            "Cannot set sample format to 16-bit little endian! (%d, %s)\n");

  // Set buffer size
  alsaCheck(snd_pcm_hw_params_set_buffer_size(handle, hw, A_BUFSIZE),
            "Cannot set buffer size to %d frames! (%d, %s)\n", A_BUFSIZE);

  // Set access mode
  alsaCheck(snd_pcm_hw_params_set_access(handle, hw, A_ACCESS),
            "Cannot set access mode to interleaved samples! (%d, %s)\n");

  // Apply hardware parameters
  alsaCheck(snd_pcm_hw_params(handle, hw),
            "Cannot apply hardware configuration! (%d, %s)\n");

  // Write dummy frames
  alsaCheck(snd_pcm_writei(handle, a_samples, A_BUFSIZE),
            "Error playing dummy samples! (%d, %s)\n");

  // ALSA successfully initialized, signal main thread
  pthread_cond_signal(&a_c);
  return handle;

#undef alsaCheck
#undef alsaCheckNoClose
#undef condCheck
}

// Audio thread entry point
static void *a_main(void *unused) {
  snd_pcm_t *handle;
  snd_pcm_sframes_t frames;

  (void)unused;

  // Initialize ALSA
  handle = a_init();
  if (!handle) pthread_exit(NULL);

  // Start sound loop
  snd_pcm_prepare(handle);

  for (;;) {
    pthread_mutex_lock(&a_m);

    if (a_quit) break;

    if (a_bgm) ReadADPCM((audio_frame_t*)a_samples, A_BUFSIZE);
    else memset(a_samples, 0, 2*A_CHANNELS*A_BUFSIZE);

    // Mix all sound channels
    i32 left, right;
    for (uptr i = 0; i < SND_CHANNELS; ++i) {
      if (s_channels[i].playing()) {
        for (uptr j = s_channels[i].startFrame; j < A_BUFSIZE; ++j) {
          if (s_channels[i].p == s_channels[i].end) {
            s_channels[i].p = NULL; // Channel isn't playing a sound anymore
            break;
          }

          left = a_samples[2*j] + s_channels[i].p->left;
          right = a_samples[2*j+1] + s_channels[i].p->right;

          if (left > 32767) left = 32767;
          else if (left < -32768) left = -32768;

          if (right > 32767) right = 32767;
          else if (right < -32768) right = -32768;

          a_samples[2*j] = left;
          a_samples[2*j+1] = right;

          ++s_channels[i].p;
        }

        // When we continue playing this sound, we don't wanna
        // start it at an offset
        s_channels[i].startFrame = 0;
      }
    }

    // Time when play started
    a_bufPlayTime = GetTime();
    pthread_mutex_unlock(&a_m);

    frames = snd_pcm_writei(handle, a_samples, A_BUFSIZE);

    if (frames == -EPIPE) { // Underrun
      puts("Underrun");
      snd_pcm_prepare(handle);
    } else if (frames < 0) {
      // Irrecoverable error occurred, error out
      snd_pcm_drain(handle);

      pthread_mutex_lock(&a_m);
      sprintf(a_errbuf, "Error playing samples! (%d, %s)\n", (int)frames, snd_strerror(frames));
      pthread_mutex_unlock(&a_m);
      snd_pcm_close(handle);
      pthread_exit(NULL);
    }
  }

  // After setting a_quit, the main thread waits for this thread to shut down
  snd_pcm_drain(handle);
  pthread_mutex_unlock(&a_m);
  snd_pcm_close(handle);
  pthread_exit(NULL);
}

// Is sound system initialized?
static inline bptr IsInitted() {
  return (bptr)a_samples;
}

// Load sounds into s_sounds and s_soundBuf
void LoadSounds() {
  uptr soundBufSize = A_SOUND_BUF_SIZE;
  audio_frame_t *p;

  // Allocate sound buffer
  p = s_soundBuf = (audio_frame_t*)Alloc(soundBufSize);

  // Read raw sound data into sound buffer
  // and initialize s_sounds
  for (uptr i = 0; i < SND_COUNT; ++i) {
    // Set s_sounds start sample position
    s_sounds[i].p = p;

    // Read data into buffer and close file
    uptr frames = OpenADPCM(G_SoundNames[i]);
    if (!frames) LOG_ERROR(FMT.s("Couldn't open ").s(G_SoundNames[i]).s("!").STR);
    ReadADPCM(p, frames);
    CloseADPCM();

    p += frames;

    // Set s_sounds end sample position
    // and sound id
    s_sounds[i].end = p;
    s_sounds[i].id = i;

    ASSERT(p <= (audio_frame_t*)((u8*)s_soundBuf+A_SOUND_BUF_SIZE));
  }

  LOG_INFO(FMT.s("Sound buffer size: ").i(p-s_soundBuf).s(" frames").STR);
}

void InitAudio() {
  int err;

  LoadSounds();

  a_samples = (i16*)Alloc(sizeof(i16)*A_CHANNELS*A_BUFSIZE);

  memset(a_samples, 0, sizeof(i16)*A_CHANNELS*A_BUFSIZE);

  err = pthread_create(&a_tid, NULL, a_main, NULL);
  if (err) LOG_ERROR("Cannot create audio thread!");

  // Wait for audio thread to initialize
  pthread_cond_wait(&a_c, &a_m);

  // Check if an error occurred
  if (a_errbuf[0]) {
    LOG_INFO(a_errbuf);
    pthread_mutex_unlock(&a_m);
    LOG_ERROR("An error occurred while initializing audio thread!");
  }

  pthread_mutex_unlock(&a_m); // pthread_cond_wait automatically locks mutex
}

void FreeAudio() {
  if (!IsInitted()) return;

  a_quit = true;
  pthread_join(a_tid, NULL);
  if (a_bgm) CloseADPCM();
  pthread_cond_destroy(&a_c);
  pthread_mutex_unlock(&a_m);
  pthread_mutex_destroy(&a_m);

  Free(a_samples);
  Free(s_soundBuf);
}

void PlayBGM(const char *filename) {
  if (!IsInitted() ||
      (filename && !strcmp(filename, a_bgmName)))
    return;

  pthread_mutex_lock(&a_m);

  if (a_bgm) CloseADPCM();
  a_bgm = false;

  if (!filename ||
      !*filename ||
      !OpenADPCM(filename))
  {
    memset(a_bgmName, 0, sizeof(a_bgmName));
    pthread_mutex_unlock(&a_m);
    return;
  }

  strcpy(a_bgmName, filename);
  a_bgm = true;
  pthread_mutex_unlock(&a_m);
}

sound_handle_t PlaySound(sound_t snd) {
  if (!IsInitted()) return 0;

  pthread_mutex_lock(&a_m);

  // Play sound in first inactive channel
  uptr i;
  for (i = 0; i < SND_CHANNELS; ++i) {
    if (!s_channels[i].playing()) break;
  }

  // If there was no free channel, take the first free channel
  if (i == SND_CHANNELS) i = 0;

  // Initialize channel
  s_channels[i] = s_sounds[snd];

  // Set start frame
  s_channels[i].startFrame = (GetTime()-a_bufPlayTime)*A_SAMPLERATE/g_timerFrequency;

  pthread_mutex_unlock(&a_m);

  // If this channel stops, and start with another sound, and you
  // call StopSound on this channel handle... it'll stop the new sound
  //
  // Whatever.
  return &s_channels[i];
}

void StopAllSounds() {
  if (!IsInitted()) return;

  pthread_mutex_lock(&a_m);
  memset(s_channels, 0, sizeof(sound_channel)*SND_CHANNELS);
  pthread_mutex_unlock(&a_m);
}

void StopSound(sound_t id) {
  if (!IsInitted()) return;

  pthread_mutex_lock(&a_m);
  for (uptr i = 0; i < SND_CHANNELS; ++i)
    if (s_channels[i].id == id) s_channels[i].p = NULL; // Stop playing sound
  pthread_mutex_unlock(&a_m);
}

void StopSound(sound_handle_t handle) {
  if (!IsInitted()) return;

  pthread_mutex_lock(&a_m);
  ((sound_channel*)handle)->p = NULL;
  pthread_mutex_unlock(&a_m);
}

// Empty function on linux
void UpdateAudio() {}
