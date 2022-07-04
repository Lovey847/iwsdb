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
 * src/loveylib/posix/loveylib_posix_stream.cpp:
 *  POSIX standard input and output implementation
 *
 ************************************************************/

#include "loveylib/types.h"
#include "loveylib/stream.h"
#include "loveylib/assert.h"

#include <unistd.h>

// Dummy seeking functions since you can't seek
// in standard streams
static bfast SeekDummy(stream_t *s, iptr pos, stream_origin_t origin) {
  (void)s; (void)pos; (void)origin;
  return false;
}
static iptr TellDummy(const stream_t *s) {
  (void)s;
  return -1;
}

// Standard input
static iptr ReadInput(stream_t *s, void *out, uptr size) {
  (void)s;
  return read(STDIN_FILENO, out, size);
}

static iptr WriteInput(stream_t *s, const void *in, uptr size) {
  (void)s; (void)in; (void)size;
  return -1; // Cannot write to standard input
}

static const stream_funcs_t S_InputFuncs = {
  ReadInput, WriteInput, SeekDummy, TellDummy
};

bfast GetStandardInput(stream_t *out) {
  ASSERT(!out->open());
  out->f = &S_InputFuncs;

  return true;
}

// Standard output
static iptr ReadOutput(stream_t *s, void *out, uptr size) {
  (void)s; (void)out; (void)size;
  return -1; // Cannot read from standard output
}

static iptr WriteOutput(stream_t *s, const void *in, uptr size) {
  (void)s;
  return write(STDOUT_FILENO, in, size);
}

static const stream_funcs_t S_OutputFuncs = {
  ReadOutput, WriteOutput, SeekDummy, TellDummy
};

bfast GetStandardOutput(stream_t *out) {
  ASSERT(!out->open());
  out->f = &S_OutputFuncs;

  return true;
}
