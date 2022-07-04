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
 * src/loveylib/timer.h:
 *  Platform-dependent timer
 *
 ************************************************************/

#ifndef _LOVEYLIB_TIMER_H
#define _LOVEYLIB_TIMER_H

#include "loveylib/types.h"
#include "loveylib_config.h"

typedef u64 timestamp_t;

// Do whatever timer initialization we need to
// do before using it
//
// Returns false on failure
bfast InitTimer();

// Returns timer frequency,
// which is the number of timer clocks in a second
timestamp_t GetTimerFrequency();

// Get timer clocks since undefined point
timestamp_t GetTime();

// Sleep for specified amount of microseconds,
// may need to busy wait
void MicrosecondDelay(timestamp_t freq, u32 microseconds);

#endif //_LOVEYLIB_TIMER_H
