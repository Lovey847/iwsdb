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
 * src/loveylib/apple/loveylib_apple_timer.cpp:
 *  Apple timer implementation
 *
 ************************************************************/

#include <mach/mach_time.h>
#include <mach/mach.h>
#include <unistd.h>

#include "loveylib/timer.h"
#include "loveylib/assert.h"
#include "loveylib/utils.h"

static mach_timebase_info_data_t s_timebaseInfo = {0, 0};

#define IS_INITTED() (s_timebaseInfo.denom != 0)

bfast InitTimer() {
  mach_timebase_info(&s_timebaseInfo);
  ASSERT(IS_INITTED());

  return true;
}

timestamp_t GetTimerFrequency() {
  ASSERT(IS_INITTED());
  return 1000000000 * s_timebaseInfo.numer / s_timebaseInfo.denom;
}

timestamp_t GetTime() {
  ASSERT(IS_INITTED());
  return mach_absolute_time();
}

void MicrosecondDelay(timestamp_t freq, u32 microseconds) {
  ASSERT(IS_INITTED());
  ASSERT(freq == GetTimerFrequency());
  (void)freq;

  usleep(microseconds);
}
