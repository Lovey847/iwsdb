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
 * src/loveylib/posix/loveylib_posix_file.cpp:
 *  POSIX file I/O
 *
 ************************************************************/

#include "loveylib/types.h"
#include "loveylib/stream.h"
#include "loveylib/file.h"
#include "loveylib/assert.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

// File stream
struct file_stream_t {
  const stream_funcs_t *f;

  // File descriptor
  int fd;
};
static_assert(sizeof(file_stream_t) <= sizeof(stream_t), "");

// File stream functions
static iptr ReadFile(stream_t *s, void *out, uptr size) {
  file_stream_t * const data = (file_stream_t*)s;
  ASSERT(s->open());

  return read(data->fd, out, size);
}

static iptr WriteFile(stream_t *s, const void *in, uptr size) {
  file_stream_t * const data = (file_stream_t*)s;
  ASSERT(s->open());

  return write(data->fd, in, size);
}

static bfast SeekFile(stream_t *s, iptr pos, stream_origin_t origin) {
  file_stream_t * const data = (file_stream_t*)s;
  int whence;

  ASSERT(s->open());

  // Origin to whence conversion
  switch (origin) {
  case ORIGIN_SET: whence = SEEK_SET; break;
  case ORIGIN_END: whence = SEEK_END; break;
  case ORIGIN_CUR: whence = SEEK_CUR; break;
  }

  return lseek(data->fd, pos, whence) != (off_t)-1;
}

static iptr TellFile(const stream_t *s) {
  file_stream_t * const data = (file_stream_t*)s;
  ASSERT(s->open());
  return lseek(data->fd, 0, SEEK_CUR);
}

static const stream_funcs_t S_FileFuncs = {
  ReadFile, WriteFile, SeekFile, TellFile
};

// Open file handle
bfast OpenFile(stream_t *out, const char *name, file_mode_t mode) {
  ASSERT(!out->open());

  file_stream_t * const o = (file_stream_t*)out;

  // Convert mode to posix flags and mode
  int f, m;
  int fd;

  switch (mode) {
  case FILE_READ_ONLY:
    f = O_RDONLY;
    m = 0;
    break;
  case FILE_WRITE_ONLY:
    f = O_WRONLY | O_CREAT|O_TRUNC;
    m = 0664;
    break;
  case FILE_READ_WRITE:
    f = O_RDWR | O_CREAT;
    m = 0664;
    break;
  }

  // If open failed, return false
  fd = open(name, f, m);
  if (fd < 0) return false;

  // Fill output struct
  o->f = &S_FileFuncs;
  o->fd = fd;

  return true;
}

// Close file
void CloseFile(stream_t *f) {
  file_stream_t * const i = (file_stream_t*)f;

  ASSERT(i->f == &S_FileFuncs);

  // Close fd stored in stream
  close(i->fd);

  // Mark stream as closed
  i->f = NULL;
}
