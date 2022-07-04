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
 * This file is part of LoveyLib
 *
 * src/loveylib/posix/loveylib_posix_timer.cpp:
 *  POSIX timer implementation
 *
 ************************************************************/

// For clock_gettime and nanosleep
#define _POSIX_C_SOURCE 199309L

#include "loveylib/timer.h"
#include "loveylib/assert.h"
#include "loveylib/utils.h"
#include <time.h>

// Make sure the user calls InitTimer before using
// timer functions in the debug build
IN_DEBUG (

static bfast initted = false;
static inline void Init() {
  initted = true;
}

)

// In the release build, don't bother
IN_RELEASE (

static constexpr const bfast initted = true;
static inline void Init() {}

)

bfast InitTimer() {
  // Don't initialize timer twice!
  ASSERT(!initted);

  // No initialization done on posix, but we
  // need to make sure it's called
  Init();
  return true;
}

timestamp_t GetTimerFrequency() {
  ASSERT(initted);
  return 1000000000;
}

timestamp_t GetTime() {
  ASSERT(initted);

  struct timespec ret;

  clock_gettime(CLOCK_MONOTONIC_RAW, &ret);
  return ret.tv_sec*1000000000 + ret.tv_nsec;
}

void MicrosecondDelay(timestamp_t freq, u32 microseconds) {
  ASSERT(initted);

  (void)freq; // Timer frequency is constant in POSIX
  const struct timespec t = {microseconds/1000000, microseconds%1000000 * 1000};

  nanosleep(&t, NULL);
}
