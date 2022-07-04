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
 * src/loveylib/win32/loveylib_win32_stream.cpp:
 *  Win32 standard input and output implementation
 *
 ************************************************************/

#include "loveylib/types.h"
#include "loveylib/stream.h"
#include "loveylib/assert.h"

#include "loveylib/win32/loveylib_windows.h"

struct std_stream_t {
  const stream_funcs_t *f;

  win32::handle_t handle; // Input/output handle
};
static_assert(sizeof(std_stream_t) <= sizeof(stream_t), "");

static iptr ReadStd(stream_t *s, void *out, uptr size) {
  std_stream_t * const data = (std_stream_t*)s;

  u32 ret;
  if (!win32::ReadConsole(data->handle, out, size, &ret, NULL))
    return -1;

  return ret;
}

static iptr WriteStd(stream_t *s, const void *in, uptr size) {
  std_stream_t * const data = (std_stream_t*)s;

  u32 ret;
  if (!win32::WriteConsole(data->handle, in, size, &ret, NULL))
    return -1;

  return ret;
}

static bfast SeekStd(stream_t *s, iptr pos, stream_origin_t origin) {
  (void)s; (void)pos; (void)origin;
  return false; // Standard streams don't have offsets
}

static iptr TellStd(const stream_t *s) {
  (void)s;
  return -1; // Standard streams don't have offsets
}

static const stream_funcs_t S_StdFuncs = {
  ReadStd, WriteStd, SeekStd, TellStd
};

static bfast GetStandardStream(stream_t *out, u32 stdHandle) {
  ASSERT(!out->f);

  win32::handle_t h = win32::GetStdHandle(stdHandle);
  if ((h == NULL) ||
      (h == win32::INVALID_HANDLE_VALUE))
    return false;

  std_stream_t * const o = (std_stream_t*)out;

  o->f = &S_StdFuncs;
  o->handle = h;

  return true;
}

bfast GetStandardInput(stream_t *out) {
  return GetStandardStream(out, win32::STD_INPUT_HANDLE);
}

bfast GetStandardOutput(stream_t *out) {
  return GetStandardStream(out, win32::STD_OUTPUT_HANDLE);
}
