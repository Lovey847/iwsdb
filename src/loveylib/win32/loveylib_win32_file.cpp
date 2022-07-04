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
 * src/loveylib/win32/loveylib_win32_file.cpp:
 *  Win32 file I/O
 *
 ************************************************************/

#include "loveylib/types.h"
#include "loveylib/stream.h"
#include "loveylib/file.h"
#include "loveylib/assert.h"

#include "loveylib/win32/loveylib_windows.h"

// File stream
struct file_stream_t {
  const stream_funcs_t *f;

  // File handle
  win32::handle_t handle;
};
static_assert(sizeof(file_stream_t) <= sizeof(stream_t), "");

// File stream functions
static iptr ReadFile(stream_t *s, void *out, uptr size) {
  file_stream_t * const data = (file_stream_t*)s;
  ASSERT(s->open());

  u32 ret;
  const b32 status = win32::ReadFile(data->handle, out, size, &ret, NULL);

  return status ? ret : -1;
}

static iptr WriteFile(stream_t *s, const void *in, uptr size) {
  file_stream_t * const data = (file_stream_t*)s;
  ASSERT(s->open());

  u32 ret;
  const b32 status = win32::WriteFile(data->handle, in, size, &ret, NULL);

  return status ? ret : -1;
}

static bfast SeekFile(stream_t *s, iptr pos, stream_origin_t origin) {
  file_stream_t * const data = (file_stream_t*)s;
  u32 moveMethod;

  ASSERT(s->open());

  switch (origin) {
  case ORIGIN_SET: moveMethod = win32::FILE_BEGIN; break;
  case ORIGIN_CUR: moveMethod = win32::FILE_CURRENT; break;
  case ORIGIN_END: moveMethod = win32::FILE_END; break;
  }

  const b32 status =
    win32::SetFilePointerEx(data->handle, pos, NULL, moveMethod);

  return status;
}

static iptr TellFile(const stream_t *s) {
  file_stream_t * const data = (file_stream_t*)s;
  u64 ret;

  ASSERT(s->open());

  const b32 status =
    win32::SetFilePointerEx(data->handle, 0, &ret, win32::FILE_CURRENT);

  return status ? ret : -1;
}

static const stream_funcs_t S_FileFuncs = {
  ReadFile, WriteFile, SeekFile, TellFile
};

// Open file handle
bfast OpenFile(stream_t *out, const char *name, file_mode_t mode) {
  ASSERT(!out->open());

  file_stream_t * const o = (file_stream_t*)out;

  // Convert mode to file flags
  u32 desiredAccess, creationDisposition;

  switch (mode) {
  case FILE_READ_ONLY:
    desiredAccess = win32::GENERIC_READ;
    creationDisposition = win32::OPEN_EXISTING;
    break;
  case FILE_WRITE_ONLY:
    desiredAccess = win32::GENERIC_WRITE;
    creationDisposition = win32::CREATE_ALWAYS;
    break;
  case FILE_READ_WRITE:
    desiredAccess = win32::GENERIC_READ|win32::GENERIC_WRITE;
    creationDisposition = win32::OPEN_ALWAYS;
    break;
  }

  // If file creation fails, return false
  win32::handle_t handle =
    win32::CreateFile(name,
                      desiredAccess, 0, NULL,
                      creationDisposition, win32::FILE_ATTRIBUTE_NORMAL, NULL);
  if (handle == win32::INVALID_HANDLE_VALUE) return false;

  // Otherwise, fill output struct and return true
  o->f = &S_FileFuncs;
  o->handle = handle;

  return true;
}

// Close file handle
void CloseFile(stream_t *f) {
  file_stream_t * const i = (file_stream_t*)f;

  ASSERT(i->f == &S_FileFuncs);

  // Close file handle
  win32::CloseHandle(i->handle);

  // Mark stream as closed
  i->f = NULL;
}
