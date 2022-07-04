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
 * src/loveylib/win32/loveylib_win32_timer.cpp:
 *  Win32 timer implementation
 *
 ************************************************************/

#include "loveylib/types.h"
#include "loveylib/timer.h"
#include "loveylib/assert.h"
#include "loveylib/utils.h"

#include "loveylib/win32/loveylib_windows.h"

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
  // Set windows scheduler granularity
  win32::TimeBeginPeriod(1);
  Init();
  return true;
}

timestamp_t GetTimerFrequency() {
  ASSERT(initted);

  timestamp_t ret;

  // Get global frequency
  win32::QueryPerformanceFrequency(&ret);

  return ret;
}

timestamp_t GetTime() {
  ASSERT(initted);

  timestamp_t ret;
  win32::QueryPerformanceCounter(&ret);
  return ret;
}

void MicrosecondDelay(timestamp_t freq, u32 microseconds) {
  ASSERT(initted);

  const timestamp_t start = GetTime();

  // Sleep for milliseconds, add some margin
  i32 milliseconds = (i32)(microseconds/1000) - 1;
  if (milliseconds >= 1) win32::Sleep(milliseconds);

  // Busy wait the rest

  // Convert from microseconds to clocks
  microseconds = microseconds*freq/1000000;

  // Start busy wait
  while (GetTime()-start < microseconds);
}
