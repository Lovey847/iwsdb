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

#include "audio.h"
#include "log.h"
#include "loveylib/file.h"
#include "loveylib/win32/loveylib_windows.h"
#include "loveylib/timer.h"
#include "loveylib/assert.h"
#include "str.h"
#include "mem.h"

#include <mmdeviceapi.h>
#include <audioclient.h>
#include <initguid.h>

#include <cstring>

extern timestamp_t g_timerFrequency; // From main.cpp
#define TIMER_FREQUENCY g_timerFrequency

static constexpr const u32 BUFTIME = 10*1000*1000*2/60;
static constexpr const uptr SOUND_BUF_SIZE = 6873504;

static IMMDeviceEnumerator *m_enumerator;
static IMMDevice *m_device;
static IAudioClient *m_client;
static IAudioRenderClient *m_renderClient;
static UINT32 m_bufSize;

static const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
static const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
static const IID IID_IAudioClient = __uuidof(IAudioClient);
static const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

static bfast s_bgm; // Background music file handle

static char s_bgmName[64];

struct sound_channel {
  // If p == NULL, this sound channel is inactive
  audio_frame_t *p, *end;
  sound_t id;

  inline bptr playing() const {return (bptr)p;}
};

static sound_channel s_sounds[SND_COUNT];
static audio_frame_t *s_soundBuf;

static constexpr const uptr SND_CHANNELS = 16;
static sound_channel s_channels[SND_CHANNELS];

// Load sounds into s_sounds and s_soundBuf
void LoadSounds() {
  uptr soundBufSize = SOUND_BUF_SIZE;
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

    ASSERT(p <= (audio_frame_t*)((u8*)s_soundBuf+SOUND_BUF_SIZE));
  }

  LOG_INFO(FMT.s("Sound buffer size: ").i(p-s_soundBuf).s(" frames").STR);
}

void InitAudio() {
  LoadSounds();

  // Fill out audio stream format
  WAVEFORMATEX fmt, *fmtPtr;
  HRESULT result;

  fmt.wFormatTag = WAVE_FORMAT_PCM;
  fmt.nChannels = 2;
  fmt.nSamplesPerSec = 48000;
  fmt.wBitsPerSample = 16;
  fmt.nAvgBytesPerSec = 2*2*48000;
  fmt.nBlockAlign = 2*2;
  fmt.cbSize = 0;

  // Initialize COM objects
  result = CoInitializeEx(NULL, 0);
  if (result != S_OK)
    LOG_ERROR("Cannot initialize COM library!");

  // Get reference to IMDeviceEnumerator interface
  result = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL,
                            CLSCTX_ALL, IID_IMMDeviceEnumerator,
                            (void**)&m_enumerator);
  if (result != S_OK)
    LOG_ERROR("Cannot get reference to IMMDeviceEnumerator!");

  // Get reference to IMMDevice interface from IMMDeviceEnumerator interface
  result = m_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_device);
  if (result != S_OK)
    LOG_ERROR("Cannot get reference to IMMDevice!");

  // Get reference to IAudioClient interface from IMMDevice interface
  result = m_device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&m_client);
  if (result != S_OK)
    LOG_ERROR("Cannot get reference to IAudioClient!");

  // Check if audio client supports format
  result = m_client->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, &fmt, &fmtPtr);
  if (result != S_OK)
    LOG_ERROR("Stereo 16-bit PCM at 48khz isn't supported by audio engine!");

  // Initialize IAudioClient interface
  result = m_client->Initialize(AUDCLNT_SHAREMODE_SHARED, 0,
                                BUFTIME, 0, &fmt, NULL);
  if (result != S_OK)
    LOG_ERROR("Cannot initialize IAudioClient!");

  // Get buffer size
  result = m_client->GetBufferSize(&m_bufSize);
  if (result != S_OK)
    LOG_ERROR("Cannot get buffer size!");

  // Get reference to IAudioRenderClient interface from IAudioClient interface
  result = m_client->GetService(IID_IAudioRenderClient, (void**)&m_renderClient);
  if (result != S_OK)
    LOG_ERROR("Cannot get reference to IAudioRenderClient!");

  // Fill buffer with initial audio data
  u16 *samples;
  result = m_renderClient->GetBuffer(m_bufSize, (BYTE**)&samples);
  if (result != S_OK)
    LOG_ERROR("Cannot get buffer from audio stream!");

  memset(samples, 0, 2*2*m_bufSize);

  result = m_renderClient->ReleaseBuffer(m_bufSize, 0);
  if (result != S_OK)
    LOG_ERROR("Cannot release buffer of audio stream!");

  // Start audio playback
  result = m_client->Start();
  if (result != S_OK)
    LOG_ERROR("Cannot start audio playback!");
}
void FreeAudio() {
  // Wait for buffer to finish playing audio data
  Sleep((float)m_bufSize * (1.f/48.f));

  // Stop audio playback
  m_client->Stop();

  // Release all audio interfaces
  m_enumerator->Release();
  m_device->Release();
  m_client->Release();
  m_renderClient->Release();

  // Uninitialize COM library
  CoUninitialize();
}

void PlayBGM(const char *filename) {
  if (filename && !strcmp(filename, s_bgmName)) return;
  if (s_bgm) CloseADPCM();
  s_bgm = false;

  if (!filename ||
      !*filename ||
      !OpenADPCM(filename))
  {
    memset(s_bgmName, 0, sizeof(s_bgmName));
    return;
  }

  strcpy(s_bgmName, filename);
  s_bgm = true;
}

#undef PlaySound
sound_handle_t PlaySound(sound_t snd) {
  // Play sound in first inactive channel
  uptr i;
  for (i = 0; i < SND_CHANNELS; ++i) {
    if (!s_channels[i].playing()) break;
  }

  // If there was no free channel, take the first free channel
  if (i == SND_CHANNELS) i = 0;

  // Initialize channel
  s_channels[i] = s_sounds[snd];

  // If this channel stops, and start with another sound, and you
  // call StopSound on this channel handle... it'll stop the new sound
  //
  // Whatever.
  return &s_channels[i];
}

void StopAllSounds() {
  memset(s_channels, 0, sizeof(sound_channel)*SND_CHANNELS);
}
void StopSound(sound_t id) {
  for (uptr i = 0; i < SND_CHANNELS; ++i)
    if (s_channels[i].id == id) s_channels[i].p = NULL; // Stop playing sound
}
void StopSound(sound_handle_t handle) {
  ((sound_channel*)handle)->p = NULL;
}

void UpdateAudio() {
  // Write samples to audio stream
  UINT32 bufSize;
  HRESULT result;
  result = m_client->GetCurrentPadding(&bufSize);
  if (result != S_OK)
    LOG_ERROR("Cannot get buffer padding!");

  // Get buffer size from padding
  bufSize = m_bufSize - bufSize;

  // Get buffer
  i16 *samples;
  result = m_renderClient->GetBuffer(bufSize, (BYTE**)&samples);
  if (result != S_OK)
    LOG_ERROR("Cannot get sample buffer!");

  // Write BGM
  if (s_bgm) ReadADPCM((audio_frame_t*)samples, bufSize);
  else memset(samples, 0, 2*2*bufSize);

  // Mix all sound channels
  i32 left, right;
  for (uptr i = 0; i < SND_CHANNELS; ++i) {
    if (s_channels[i].playing()) {
      for (uptr j = 0; j < bufSize; ++j) {
        if (s_channels[i].p == s_channels[i].end) {
          s_channels[i].p = NULL; // Channel isn't playing a sound anymore
          break;
        }

        left = samples[2*j] + s_channels[i].p->left;
        right = samples[2*j+1] + s_channels[i].p->right;

        if (left > 32767) left = 32767;
        else if (left < -32768) left = -32768;

        if (right > 32767) right = 32767;
        else if (right < -32768) right = -32768;

        samples[2*j] = left;
        samples[2*j+1] = right;

        ++s_channels[i].p;
      }
    }
  }

  m_renderClient->ReleaseBuffer(bufSize, 0);
}
