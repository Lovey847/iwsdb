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
 * src/loveylib/file.h:
 *  File I/O
 *
 ************************************************************/

#ifndef _LOVEYLIB_FILE_H
#define _LOVEYLIB_FILE_H

#include "loveylib/types.h"
#include "loveylib/stream.h"

// File open modes
enum file_mode_e : ufast {
  // Open the file as read-only
  FILE_READ_ONLY = 0,

  // Create a file and open as write-only,
  // if the file exists, truncate it to 0 bytes
  FILE_WRITE_ONLY,

  // Create or open a file as read and write,
  // NOTE: If the file exists, it is not truncated,
  //       and the stream pointer is put at the
  //       beginning of the file
  FILE_READ_WRITE,
};
typedef ufast file_mode_t;

// Open file stream
// Returns false on error
bfast OpenFile(stream_t *out, const char *name, file_mode_t mode);

// Close file stream
void CloseFile(stream_t *f);

#endif //_LOVEYLIB_FILE_H
