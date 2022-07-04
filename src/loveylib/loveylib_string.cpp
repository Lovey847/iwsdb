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
 * src/loveylib/loveylib_string.cpp:
 *  String utilities
 *
 ************************************************************/

#include "loveylib/types.h"
#include "loveylib/string.h"
#include "loveylib/assert.h"

#include <cstring>

// Get number length
// Only accepts positive integers
uptr NumberLength(u64 n) {
  uptr ret = 1;
  u64 pow10 = 10;

  while (pow10 <= n) {
    ++ret;
    pow10 *= 10;
  }

  return ret;
}

char *IntegerToString(char *out, i64 num) {
  if (num < 0) {
    *out++ = '-';
    num = -num;
  }

  const uptr len = NumberLength(num);

  // Set NULL terminator
  out[len] = 0;

  // Loop over digits in number, writing to out
  for (uptr i = len; i--; num /= 10)
    out[i] = num%10 + '0';

  return out+len;
}

char *IntegerToString(char *out, i64 num, uptr len) {
  if (num < 0) {
    *out++ = '-';
    num = -num;
  }

  // Set NULL terminator
  out[len] = 0;

  // Loop over digits in number, writing to out
  for (uptr i = len; i--; num /= 10)
    out[i] = num%10 + '0';

  return out+len;
}

i64 StringToInteger(const char *str) {
  i64 curPow10 = 1, ret = 0;

  // Check if integer is negative
  if (*str == '-') {
    curPow10 = -1;
    ++str;
  }

  // Loop over digits in str, filling ret
  for (uptr i = strlen(str); i--; curPow10 *= 10) {
    ASSERT((str[i] <= '9') && (str[i] >= '0'));
    ret += (i64)(str[i]-'0') * curPow10;
  }

  return ret;
}

char *FloatToString(char *out, f32 num) {
  if (num < 0.f) {
    *out++ = '-';
    num = -num;
  }

  const i64 integer = num;
  const i64 decimal = (num-(f32)integer)*1000000.f;

  // Write integer part of number
  out = IntegerToString(out, integer);

  // Write decimal part of number
  *out = '.';
  return IntegerToString(out+1, decimal, 6);
}

f32 StringToFloat(const char *str) {
  f32 curPow10, ret = 0.f, sign = 1.f;

  // Check if number is negative
  if (*str == '-') {
    sign = -1.f;
    ++str;
  }

  curPow10 = sign;

  // Loop over digits in str, filling ret
  for (uptr i = strlen(str); i--; curPow10 *= 10.f) {
    if (str[i] == '.') {
      ASSERT(curPow10*sign > 1.f); // "106." isn't allowed

      // Put all digits behind decimal point
      ret /= curPow10*sign;

      // Reset current power of 10
      curPow10 = sign;

      // If we're at the end of the string,
      // stop processing
      if (!i) break;
      else --i;
    }

    ASSERT((str[i] <= '9') && (str[i] >= '0'));

    ret += (f32)(str[i]-'0') * curPow10;
  }

  return ret;
}
