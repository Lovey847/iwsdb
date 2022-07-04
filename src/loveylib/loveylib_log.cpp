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
 * src/loveylib/loveylib_log.cpp:
 *  Information logging
 *
 ************************************************************/

#include "loveylib/types.h"
#include "loveylib/stream.h"
#include "loveylib/file.h"
#include "loveylib/log.h"
#include "loveylib/string.h"

#include <cstring>

bfast OpenDefaultLogStreams(log_streams_t s) {
  bfast ret = false;

  // Stream 1 is standard output
  ret = GetStandardOutput(&s[0]);

  // Stream 2 is a file
  ret |= OpenFile(&s[1], "log.txt", FILE_WRITE_ONLY);

  // If neither are open, return false
  return ret;
}

void CloseDefaultLogStreams(log_streams_t s) {
  CloseFile(&s[1]);
}

void LogString(log_streams_t s, const char *str) {
  // Fill buf with str + "\n"
  char buf[256];

  // 256-1 so the following newline addition works
  strncpy(buf, str, 255);

  const uptr bufLen = strlen(buf)+1;
  buf[bufLen-1] = '\n';
  buf[bufLen] = 0;

  // Write buf to all streams in array
  for (stream_t *i = s+MAX_LOG_STREAMS; i-- != s;)
    if (i->open()) i->f->write(i, buf, bufLen);
}

void LogInfoExplicit(log_streams_t s, const char *file, int line, const char *str) {
  // Fill buf with file + ", " + str(line) + ": " + str + "\n"
  char buf[512];

  const uptr bufLen =
    format_buf_t(buf).s(file).s(", ").i(line).s(": ").s(str).s("\n").p - buf;

  // Write buf to all streams in array
  for (stream_t *i = s+MAX_LOG_STREAMS; i-- != s;)
    if (i->open()) i->f->write(i, buf, bufLen);
}
