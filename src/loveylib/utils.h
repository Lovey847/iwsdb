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
 * src/loveylib/utils.h:
 *  Various miscellaneous utilities that help out when
 *  programming, if a group of utilities can be categorized
 *  than they should be taken out of this file
 *
 ************************************************************/

#ifndef _LOVEYLIB_UTILS_H
#define _LOVEYLIB_UTILS_H

#include "loveylib/types.h"

// Code that's only emitted in the debug
// and release builds
#ifndef NDEBUG

#define IN_DEBUG(...) __VA_ARGS__
#define IN_RELEASE(...)

#else

#define IN_DEBUG(...)
#define IN_RELEASE(...) __VA_ARGS__

#endif

// NOTE: I just have a sneaking suspicion this
//       C++11 stuff is gonna stab me in the back later.
//
// Get number of elements in array
template<typename T, uptr N>
static constexpr uptr ArraySize(const T (&)[N]) {
  return N;
}

// Get end of array
template<typename T, uptr N>
static constexpr const T *ArrayEnd(const T (&arr)[N]) {
  return arr+N;
}

// Align number down to boundary mask
template<typename T, typename T2>
static constexpr T AlignDownMask(T a, T2 b) {
  return a&~b;
}

// Align number up to boundary mask
template<typename T, typename T2>
static constexpr T AlignUpMask(T a, T2 b) {
  return (T)AlignDownMask(a+b, b);
}

// Align number down to power of 2 boundary
template<typename T, typename T2>
static constexpr T AlignDownPow2(T a, T2 b) {
  return (T)AlignDownMask(a, b-1);
}

// Align number up to power of 2 boundary
template<typename T, typename T2>
static constexpr T AlignUpPow2(T a, T2 b) {
  return (T)AlignUpMask(a, b-1);
}

// Divide integers, round result
template<typename T, typename T2>
static constexpr T RoundDiv(T a, T2 b) {
  return (a+(b>>1))/b;
}

// Divide integers, ceiling result
template<typename T, typename T2>
static constexpr T CeilDiv(T a, T2 b) {
  return (a+(b-1))/b;
}

#endif //_LOVEYLIB_UTILS_H
