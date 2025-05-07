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
#include "loveylib/timer.h"
#include "loveylib/assert.h"
#include "str.h"
#include "mem.h"

#import <Cocoa/Cocoa.h>
#import <AudioToolbox/AudioQueue.h>

// Size of audio buffers for audio queue
static constexpr const iptr AUDIO_BUF_SIZE = 4096;
// Number of audio buffers in audio queue
static constexpr const int NUM_BUFFERS = 2;

// Size of sound sample buffer
static constexpr const iptr SOUND_BUF_SIZE = 6873504;

// Audio data mutex
static NSLock *s_audioLock;
// Audio queue
static AudioQueueRef s_audioQueue;
// Audio queue buffers
static AudioQueueBufferRef s_audioBuffers[NUM_BUFFERS];

// Whether or not bgm is playing
static bfast s_bgmPlaying = false;
// Filename of bgm that's playing
static char s_bgmName[64];

struct sound_channel {
  // If p == NULL, this sound channel is inactive
  audio_frame_t *p, *end;
  sound_t id;

  inline bptr playing() const {return (bptr)p;}
};

// Sound array, contains the initial state for a sound channel that plays this
// sound.
static sound_channel s_sounds[SND_COUNT];
// Sound sample buffer
static audio_frame_t *s_soundBuf;

// Sound channel array.  All of these are mixed into the audio output stream
// along with the BGM.  When PlaySound is called, a sound channel is allocated
// to play the specified sound, and a handle to that sound channel is given
// to whatever called PlaySound.
static constexpr const uptr SND_CHANNELS = 16;
static sound_channel s_channels[SND_CHANNELS];

// Load sounds into s_sounds and s_soundBuf
// Called with s_audioLock locked
static void LoadSounds() {
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

// Mix BGM and sound channels into samples
// Called with s_audioLock locked
static void MixAudio(i16 *samples, uptr bufSize) {
  // Write BGM
  if (s_bgmPlaying) ReadADPCM((audio_frame_t*)samples, bufSize);
  else memset(samples, 0, 2 * 2 * bufSize);

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

        // Clamp left and right channels
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
}

// Audio output callback for audio queue
// Called in a separate thread after queue reads buf into the output stream
static void QueueOutputCallback(void *data, AudioQueueRef queue,
				AudioQueueBufferRef buf)
{
  i16 *samples;

  (void)data;

  [s_audioLock lock];

  samples = (i16*)buf->mAudioData;

  buf->mAudioDataByteSize = buf->mAudioDataBytesCapacity;

  // Mix sound channels into buffer
  MixAudio(samples, buf->mAudioDataByteSize / 4);

  // Queue buffer for playback in audio queue
  AudioQueueEnqueueBuffer(queue, buf, 0, NULL);

  [s_audioLock unlock];
}

// Initialize audio queue buffer
static AudioQueueBufferRef InitBuffer(AudioQueueRef queue) {
  OSStatus res;
  AudioQueueBufferRef ret;

  res = AudioQueueAllocateBuffer(queue, AUDIO_BUF_SIZE, &ret);
  if (res < 0) return nil;

  // Queue initial buffer for playback
  ret->mAudioDataByteSize = ret->mAudioDataBytesCapacity;
  memset(ret->mAudioData, 0, ret->mAudioDataByteSize);

  res = AudioQueueEnqueueBuffer(queue, ret, 0, NULL);
  if (res < 0) {
    AudioQueueFreeBuffer(queue, ret);
    return nil;
  }

  return ret;
}

// Initialize and start audio queue
void InitAudio() {
  OSStatus res;
  AudioStreamBasicDescription desc;
  iptr i;

  s_audioLock = [[NSLock alloc] init];

  memset(&desc, 0, sizeof(AudioStreamBasicDescription));

  // 48khz s16le pcm
  desc.mSampleRate		= 48000;
  desc.mFormatID		= kAudioFormatLinearPCM;
  desc.mFormatFlags		= (kLinearPCMFormatFlagIsPacked |
				   kLinearPCMFormatFlagIsSignedInteger);
  desc.mBytesPerPacket		= 4;
  desc.mFramesPerPacket		= 1;
  desc.mBytesPerFrame		= 4;
  desc.mChannelsPerFrame	= 2;
  desc.mBitsPerChannel		= 16;

  [s_audioLock lock];

  // Create audio queue
  res = AudioQueueNewOutput(&desc, QueueOutputCallback, NULL,
			    nil, NULL, 0, &s_audioQueue);
  if (res < 0) LOG_ERROR("Couldn't initialize audio mixer!");

  // Initialize all buffers
  for (i = 0; i < NUM_BUFFERS; ++i) {
    s_audioBuffers[i] = InitBuffer(s_audioQueue);

    if (!s_audioBuffers[i])
      LOG_ERROR("Couldn't initialize audio buffer!");
  }

  res = AudioQueueStart(s_audioQueue, NULL);
  if (res < 0)
    LOG_ERROR("Couldn't start audio queue!");

  LoadSounds();

  [s_audioLock unlock];
}

void FreeAudio() {
  uptr i;

  [s_audioLock lock];

  if (s_audioQueue != nil) {
    for (i = 0; i < NUM_BUFFERS; ++i)
      AudioQueueFreeBuffer(s_audioQueue, s_audioBuffers[i]);

    // QueueOutputCallback may be called in AudioQueueDispose, so make sure the
    // mutex is unlocked before then
    [s_audioLock unlock];

    AudioQueueDispose(s_audioQueue, true);

    if (s_bgmPlaying) CloseADPCM();
    s_bgmPlaying = false;
  } else {
    [s_audioLock unlock];
  }

  [s_audioLock release];
}

void PlayBGM(const char *filename) {
  if (filename && !strcmp(filename, s_bgmName)) return;

  [s_audioLock lock];

  if (s_bgmPlaying) CloseADPCM();
  s_bgmPlaying = false;

  if (!filename ||
      !*filename ||
      !OpenADPCM(filename))
  {
    memset(s_bgmName, 0, sizeof(s_bgmName));
    [s_audioLock unlock];
    return;
  }

  strncpy(s_bgmName, filename, sizeof(s_bgmName) - 1);
  s_bgmName[sizeof(s_bgmName) - 1] = 0;

  s_bgmPlaying = true;

  [s_audioLock unlock];
}

sound_handle_t PlaySound(sound_t snd) {
  [s_audioLock lock];

  // Play sound in first inactive channel
  uptr i;
  for (i = 0; i < SND_CHANNELS; ++i) {
    if (!s_channels[i].playing()) break;
  }

  // If there was no free channel, take the first free channel
  if (i == SND_CHANNELS) i = 0;

  // Initialize channel
  s_channels[i] = s_sounds[snd];

  [s_audioLock unlock];

  // If this channel stops, and start with another sound, and you
  // call StopSound on this channel handle... it'll stop the new sound
  //
  // Whatever.
  return &s_channels[i];
}

void StopAllSounds() {
  [s_audioLock lock];
  memset(s_channels, 0, sizeof(sound_channel)*SND_CHANNELS);
  [s_audioLock unlock];
}
void StopSound(sound_t id) {
  [s_audioLock lock];

  for (uptr i = 0; i < SND_CHANNELS; ++i)
    if (s_channels[i].id == id) s_channels[i].p = NULL; // Stop playing sound

  [s_audioLock unlock];
}
void StopSound(sound_handle_t handle) {
  [s_audioLock lock];
  ((sound_channel*)handle)->p = NULL;
  [s_audioLock unlock];
}

// Empty function on mac
void UpdateAudio() {}
