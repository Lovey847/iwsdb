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
 * src/loveylib/string.h:
 *  String utilities
 *
 ************************************************************/

#ifndef _LOVEYLIB_STRING_H
#define _LOVEYLIB_STRING_H

#include "loveylib/types.h"

#include <cstring>

// Get number length
uptr NumberLength(u64 num);

// Convert integer to string
// Returns pointer to end of number
char *IntegerToString(char *out, i64 num);
char *IntegerToString(char *out, i64 num, uptr len); // Left padded with 0's

// Convert string to integer, undefined return value
// if string is invalid
i64 StringToInteger(const char *str);

// Convert float to string
// Returns pointer to end of number
char *FloatToString(char *out, f32 num);

// Convert string to float, undefined return value
// if string is invalid
f32 StringToFloat(const char *str);

// String formatting
struct format_buf_t {
  char *p;

  constexpr format_buf_t(char *out) : p(out) {}

  // Gives string NULL terminator
  //
  // Should be called before giving string
  // to another function
  //
  // ret must be out from the original
  // format_buf_t construction
  inline char *str(char *ret) {
    *p = 0;
    return ret;
  }

  inline format_buf_t &i(i64 v) {
    p = IntegerToString(p, v);
    return *this;
  }

  inline format_buf_t &f(f32 v) {
    p = FloatToString(p, v);
    return *this;
  }

  inline format_buf_t &s(const char *v) {
    const uptr len = strlen(v);
    memcpy(p, v, len);
    p += len;
    return *this;
  }

  template<uptr N>
  inline format_buf_t &s(const char (&v)[N]) {
    memcpy(p, v, N-1);
    p += N-1;
    return *this;
  }

  inline format_buf_t &s(char v) {
    *p++ = v;
    return *this;
  }
};

#endif //_LOVEYLIB_STRING_H
