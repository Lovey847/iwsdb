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
 * src/loveylib/stream.h:
 *  Input output streams, can be initialized with anything
 *
 ************************************************************/

#ifndef _LOVEYLIB_STREAM_H
#define _LOVEYLIB_STREAM_H

#include "loveylib/types.h"

// Seek origin
enum stream_origin_e : ufast {
  ORIGIN_SET = 0,
  ORIGIN_END,
  ORIGIN_CUR
};
typedef ufast stream_origin_t;

// Stream struct forward declaration
struct stream_t;

// Stream function pointer types
typedef iptr (*stream_read_func_t)(stream_t */*s*/,
                                   void */*out*/, uptr /*size*/);
typedef iptr (*stream_write_func_t)(stream_t */*s*/,
                                    const void */*in*/, uptr /*size*/);
typedef bfast (*stream_seek_func_t)(stream_t */*s*/, iptr /*pos*/,
                                    stream_origin_t /*origin*/);
typedef iptr (*stream_tell_func_t)(const stream_t */*s*/);

// Stream function table
struct stream_funcs_t {
  // Returns number of bytes read, -1 on error
  stream_read_func_t read;

  // Returns number of bytes written, -1 on error
  stream_write_func_t write;

  // Returns false on error
  stream_seek_func_t seek;

  // Returns position in file, -1 on error
  stream_tell_func_t tell;
};

// Stream struct
static constexpr const uptr STREAM_SIZE = 32;
struct stream_t {
  // Function table, NULL if not open
  const stream_funcs_t *f;

  // Opaque stream data
  static_assert(STREAM_SIZE-sizeof(stream_funcs_t*) > 0, "");
  u8 data[STREAM_SIZE - sizeof(stream_funcs_t*)];

  // Initialize stream, must be done before opening
  //
  // You can also initialize the whole struct
  // to 0, in case you have a buffer you need to
  // fill out.
  inline void init() {f = NULL;}

  // Check if stream is open
  inline bptr open() const {return (bptr)f;}
};
static_assert(sizeof(stream_t) == STREAM_SIZE, "");

// Convenience stream functions
static inline iptr ReadStream(stream_t *s, void *out, uptr size) {
  return s->f->read(s, out, size);
}
static inline iptr WriteStream(stream_t *s, const void *in, uptr size) {
  return s->f->write(s, in, size);
}
static inline bfast SeekStream(stream_t *s, iptr pos, stream_origin_t origin) {
  return s->f->seek(s, pos, origin);
}
static inline iptr TellStream(const stream_t *s) {
  return s->f->tell(s);
}

// Get standard input stream
// Returns false if standard input isn't available
bfast GetStandardInput(stream_t *out);

// Same but standard output
bfast GetStandardOutput(stream_t *out);

#endif //_LOVEYLIB_STREAM_H
