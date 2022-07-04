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
 * src/loveylib/buffer.h:
 *  Buffer of any type
 *
 ************************************************************/

#ifndef _LOVEYLIB_BUFFER_H
#define _LOVEYLIB_BUFFER_H

#include "loveylib/types.h"
#include "loveylib/assert.h"
#include "loveylib/utils.h"

// Raw buffer of items, just contains groups of bytes
static constexpr const uptr BUFFER_ITEM_ALIGNMENT = 16;
struct alignas(BUFFER_ITEM_ALIGNMENT) buffer_t {
  // Pointer in buffer
  uptr cur;

  // Number of items in buffer
  uptr itemCount;

  // Size of item in buffer
  uptr itemSize;

  // Item activity is stored after the item buffer
  bfast *active;

  // Item buffer is stored off the end of the struct
  inline u8 *buf() {
    return (u8*)this + AlignUpMask(sizeof(buffer_t), BUFFER_ITEM_ALIGNMENT-1);
  }
  inline const u8 *buf() const {
    return (u8*)this + AlignUpMask(sizeof(buffer_t), BUFFER_ITEM_ALIGNMENT-1);
  }
};

// Typed buffer
template<typename T>
struct typed_buffer_t {
  buffer_t b;

  inline operator buffer_t*() {return (buffer_t*)this;}
  inline operator const buffer_t*() const {return (buffer_t*)this;}
};

// Buffer container
template<typename T, uptr N>
struct buffer_container_t {
  buffer_t b;

  T buf[N];
  bfast active[N];

  inline operator buffer_t*() {return (buffer_t*)this;}
  inline operator const buffer_t*() const {return (buffer_t*)this;}
};

// Initialize buffer
void InitBuffer(buffer_t *buf, uptr itemCount, uptr itemSize);

template<typename T>
static inline void InitBuffer(typed_buffer_t<T> &buf, uptr itemCount) {
  InitBuffer(buf, itemCount, sizeof(T));
}

template<typename T, uptr N>
static inline void InitBuffer(buffer_container_t<T, N> &buf) {
  InitBuffer(buf, N, sizeof(T));
}

// Get item from buffer
// Returns NULL if no item is available
u8 *GetBufferItem(buffer_t *buf);

template<typename T>
static inline T *GetBufferItem(typed_buffer_t<T> &buf) {
  return (T*)GetBufferItem((buffer_t*)&buf);
}

template<typename T, uptr N>
static inline T *GetBufferItem(buffer_container_t<T, N> &buf) {
  return (T*)GetBufferItem((buffer_t*)&buf);
}

// Check if buffer item exists
static inline bfast BufferItemExists(buffer_t *buf, u8 *item) {
  uptr ind = (item-buf->buf())/buf->itemSize;
  ASSERT(ind < buf->itemCount);
  return buf->active[ind];
}

template<typename T>
static inline bfast BufferItemExists(typed_buffer_t<T> &buf, T *item) {
  return BufferItemExists((buffer_t*)&buf, (u8*)item);
}

template<typename T, uptr N>
static inline bfast BufferItemExists(buffer_container_t<T, N> &buf, T *item) {
  return BufferItemExists((buffer_t*)&buf, (u8*)item);
}

// Free buffer item
// item pointer is invalidated after this call
//
// NOTE: This function doesn't free any resources, it just
// marks the item spot as free to use, you must take care
// of any resource management (closing the stream, freeing
// the memory) before freeing the buffer item
void FreeBufferItem(buffer_t *buf, void *item);

#endif //_LOVEYLIB_BUFFER_H
