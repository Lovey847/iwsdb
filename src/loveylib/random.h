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
 * src/loveylib/random.h:
 *  Random number generator
 *
 ************************************************************/

#ifndef _LOVEYLIB_RANDOM_H
#define _LOVEYLIB_RANDOM_H

#include "loveylib/types.h"
#include "loveylib/timer.h"

// RNG seed
typedef u64 rng_seed_t;

// Get random initial rng seed
static inline rng_seed_t RandomSeed() {
  return (rng_seed_t)GetTime();
}

// Get next seed from current seed
static inline rng_seed_t NextSeed(rng_seed_t seed) {
  seed ^= seed<<13;
  seed ^= seed>>19;
  seed ^= seed<<28;

  return seed;
}

// Advance rng seed
static inline rng_seed_t Random(rng_seed_t *seed) {
  return *seed = NextSeed(*seed);
}

#endif //_LOVEYLIB_RANDOM_H
