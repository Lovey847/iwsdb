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
#include "loveylib/file.h"
#include "mem.h"
#include "loveylib/utils.h"
#include "loveylib/endian.h"
#include "loveylib_config.h"
#include "log.h"

#include <cstring>

// List of sounds
const char * const G_SoundNames[SND_COUNT] = {
  "data/snd/shoot.wav", // SND_SHOOT
  "data/snd/jump.wav", // SND_JUMP
  "data/snd/djump.wav", // SND_DJUMP
  "data/snd/vineJump.wav", // SND_VINEJUMP
  "data/snd/death.wav", // SND_DEATH
  "data/snd/noSpell.wav", // SND_NOSPELL
  "data/snd/jumpSpell.wav", // SND_JUMPSPELL
  "data/snd/magicBall.wav", // SND_SHOOTSPELL
  "data/snd/speedSpell.wav", // SND_SPEEDSPELL
  "data/snd/getSpell.wav", // SND_GETSPELL
  "data/snd/save.wav", // SND_SAVE
  "data/snd/breakBlock.wav", // SND_BREAKBLOCK
  "data/snd/thunder.wav", // SND_THUNDER
  "data/snd/mikoo.wav", // SND_MIKOO
  "data/snd/mikooDefeated.wav", // SND_MIKOODEFEATED
};

#define MAGIC(a, b, c, d) CLITTLE_ENDIAN32((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))

static constexpr const u16 ADPCM_BLOCK_SIZE = 1024;
static constexpr const uptr ADPCM_BLOCK_FRAMES = ADPCM_BLOCK_SIZE-12;

// MS-ADPCM block
struct adpcm_block_t {
  u8 p0, p1;
  u16 d0, d1;
  i16 s10, s11;
  i16 s20, s21;

  i8 samples[ADPCM_BLOCK_FRAMES-2];

  inline void swap() {
    d0 = LittleEndian16(d0);
    d1 = LittleEndian16(d1);
    s10 = LittleEndian16(s10);
    s11 = LittleEndian16(s11);
    s20 = LittleEndian16(s20);
    s21 = LittleEndian16(s21);
  }
};

// MS-ADPCM WAVE header, packed
#if defined(LOVEYLIB_GNU) || defined(LOVEYLIB_MSVC)

#pragma pack(1)

#else

#error "Unknown packing pragma!"

#endif

struct wave_hdr_t {
  u32 riff; // == MAGIC('R', 'I', 'F', 'F')
  u32 riffSize;

  u32 wave; // == MAGIC('W', 'A', 'V', 'E')
  u32 fmt; // == MAGIC('f', 'm', 't', ' ')
  u32 fmtSize; // == 50

  u16 id; // == 2
  u16 channels; // == 2
  u32 sampleRate; // == 48000
  u32 bytesPerSec; // == 48283 (ADPCM_BLOCK_SIZE/ADPCM_BLOCK_FRAMES*48000)
  u16 blockAlign; // == ADPCM_BLOCK_SIZE
  u16 bitsPerSample; // == 4
  u16 extraSize; // == 32

  u16 samplesPerBlock; // == ADPCM_BLOCK_SIZE-14+2)
  u16 numCoeffs; // == 7

  u16 coeffList[2][7];

  u32 fact; // == MAGIC('f', 'a', 'c', 't')
  u32 factSize; // == 4
  u32 numSamples;

  u32 data; // == MAGIC('d', 'a', 't', 'a')
  u32 dataSize;

  void swap() {
    riffSize = LittleEndian32(riffSize);
    fmtSize = LittleEndian32(fmtSize);
    id = LittleEndian16(id);
    channels = LittleEndian16(channels);
    sampleRate = LittleEndian32(sampleRate);
    bytesPerSec = LittleEndian32(bytesPerSec);
    blockAlign = LittleEndian16(blockAlign);
    bitsPerSample = LittleEndian16(bitsPerSample);
    extraSize = LittleEndian16(extraSize);
    samplesPerBlock = LittleEndian16(samplesPerBlock);
    numCoeffs = LittleEndian16(numCoeffs);
    coeffList[0][0] = LittleEndian16(coeffList[0][0]);
    coeffList[0][1] = LittleEndian16(coeffList[0][1]);
    coeffList[0][2] = LittleEndian16(coeffList[0][2]);
    coeffList[0][3] = LittleEndian16(coeffList[0][3]);
    coeffList[0][4] = LittleEndian16(coeffList[0][4]);
    coeffList[0][5] = LittleEndian16(coeffList[0][5]);
    coeffList[0][6] = LittleEndian16(coeffList[0][6]);
    coeffList[1][0] = LittleEndian16(coeffList[1][0]);
    coeffList[1][1] = LittleEndian16(coeffList[1][1]);
    coeffList[1][2] = LittleEndian16(coeffList[1][2]);
    coeffList[1][3] = LittleEndian16(coeffList[1][3]);
    coeffList[1][4] = LittleEndian16(coeffList[1][4]);
    coeffList[1][5] = LittleEndian16(coeffList[1][5]);
    coeffList[1][6] = LittleEndian16(coeffList[1][6]);
    factSize = LittleEndian32(factSize);
    dataSize = LittleEndian32(dataSize);
  }
};

// Adaptation table
static const u16 S_AdaptTable[16] = {
  230, 230, 230, 230, 307, 409, 512, 614,
  768, 614, 512, 409, 307, 230, 230, 230
};

// Coefficient table
static const i32 S_CoeffTable[7][2] = {
  {256, 0}, {512, -256}, {0, 0}, {192, 64},
  {240, 0}, {460, -208}, {392, -232}
};

static audio_frame_t s_sampleBuf[ADPCM_BLOCK_FRAMES];
static uptr s_numSamples;
static stream_t s_file;

// Parse MS ADPCM block
bfast ParseADPCM() {
  adpcm_block_t block;

  if (s_file.f->read(&s_file, &block, sizeof(block)) < (iptr)sizeof(block)) return false;

  block.swap();

  ASSERT(block.p0 < 7);
  ASSERT(block.p1 < 7);

  // Process block header
  audio_frame_t *out = s_sampleBuf;
  out->left = block.s20;
  out++->right = block.s21;
  out->left = block.s10;
  out++->right = block.s11;

  // Decode ADPCM frames
  for (i8 *f = block.samples; f != ArrayEnd(block.samples); ++f) {
    // Get signed 4-bit value, really have to fight C++ here to
    // make sure it isn't unsigned
    i8 nibble = ((i8)(*f&0xf0))>>4;
    i32 p0 = (((S_CoeffTable[block.p0][0]*block.s10) +
               (S_CoeffTable[block.p0][1]*block.s20)) >> 8) + nibble*block.d0;

    if (p0 < -32768) p0 = -32768;
    else if (p0 > 32767) p0 = 32767;

    out->left = p0;

    block.s20 = block.s10;
    block.s10 = p0;

    // Take unsigned nibble value
    block.d0 = (S_AdaptTable[nibble&0xf]*block.d0) >> 8;

    // Keep delta >= 16, apparently this is called "saturation"
    if (block.d0 < 16) block.d0 = 16;

    // Get signed 4-bit value
    nibble = ((i8)((*f&0xf)<<4))>>4;
    i32 p1 = (((S_CoeffTable[block.p1][0]*block.s11) +
               (S_CoeffTable[block.p1][1]*block.s21)) >> 8) + nibble*block.d1;

    if (p1 < -32768) p1 = -32768;
    else if (p1 > 32767) p1 = 32767;

    out++->right = p1;

    block.s21 = block.s11;
    block.s11 = p1;

    block.d1 = (S_AdaptTable[nibble&0xf]*block.d1) >> 8;

    if (block.d1 < 16) block.d1 = 16;
  }

  ASSERT(out == ArrayEnd(s_sampleBuf));

  return true;
}

uptr OpenADPCM(const char *filename) {
  s_file.init();
  if (!OpenFile(&s_file, filename, FILE_READ_ONLY)) return 0;

  wave_hdr_t hdr;

  if (s_file.f->read(&s_file, &hdr, sizeof(wave_hdr_t)) < (iptr)sizeof(wave_hdr_t)) {
    CloseFile(&s_file);
    return 0;
  }

  hdr.swap();

  if ((hdr.riff != MAGIC('R', 'I', 'F', 'F')) ||
      (hdr.wave != MAGIC('W', 'A', 'V', 'E')) ||
      (hdr.fmt != MAGIC('f', 'm', 't', ' ')) ||
      (hdr.fmtSize != 50) ||
      (hdr.id != 2) ||
      (hdr.channels != 2) ||
      (hdr.sampleRate != 48000) ||
      (hdr.blockAlign != ADPCM_BLOCK_SIZE) ||
      (hdr.bitsPerSample != 4) ||
      (hdr.extraSize != 32) ||
      (hdr.samplesPerBlock != ADPCM_BLOCK_FRAMES) ||
      (hdr.numCoeffs != 7) ||
      (hdr.fact != MAGIC('f', 'a', 'c', 't')) ||
      (hdr.factSize != 4) ||
      (hdr.data != MAGIC('d', 'a', 't', 'a')))
  {
    LOG_STATUS("Invalid ADPCM file!");
    CloseFile(&s_file);
    return 0;
  }

  s_numSamples = hdr.numSamples;
  return s_numSamples;
}

static uptr s_sample = 0;
static uptr s_sampleBufPtr = ADPCM_BLOCK_FRAMES;
void CloseADPCM() {
  s_sample = 0;
  s_sampleBufPtr = ADPCM_BLOCK_FRAMES; // Force next ReadADPCM to call ParseADPCM
  CloseFile(&s_file);
}

static uptr ReadSamples(audio_frame_t *out, uptr frames) {
  uptr remainder = ADPCM_BLOCK_FRAMES-s_sampleBufPtr;
  if (remainder > s_numSamples-s_sample) remainder = s_numSamples-s_sample;

  if (frames > remainder) frames = remainder;

  memcpy(out, s_sampleBuf+s_sampleBufPtr, frames*4);
  s_sampleBufPtr += frames;
  s_sample += frames;

  return frames;
}

void ReadADPCM(audio_frame_t *out, uptr frames) {
  for (;;) {
    uptr ret = ReadSamples(out, frames);
    if (ret != frames) {
      s_sampleBufPtr = 0;
      out += ret;
      frames -= ret;
      if (!ParseADPCM()) {
        s_sample = 0;
        s_file.f->seek(&s_file, sizeof(wave_hdr_t), ORIGIN_SET);
      }
    } else return;
  }
}
