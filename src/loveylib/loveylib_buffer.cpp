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
 * src/loveylib/loveylib_buffer.cpp:
 *  Buffer of any type
 *
 ************************************************************/

#include "loveylib/types.h"
#include "loveylib/buffer.h"

#include <cstring>

void InitBuffer(buffer_t *buf, uptr itemCount, uptr itemSize) {
  buf->cur = 0;
  buf->itemCount = itemCount;
  buf->itemSize = itemSize;
  buf->active = (ufast*)(buf->buf() + itemCount*itemSize);

  // Clear the active area
  memset(buf->active, 0, sizeof(ufast)*buf->itemCount);
}

u8 *GetBufferItem(buffer_t *buf) {
  uptr origCur = buf->cur;

  // Loop 1: Go from current position to end of buffer
  while (buf->cur < buf->itemCount) {
    if (!buf->active[buf->cur]) goto l_itemFound;

    ++buf->cur;
  }

  // Loop 2: Go from beginning of buffer to previous current position
  buf->cur = 0;
  while (buf->cur < origCur) {
    if (!buf->active[buf->cur]) goto l_itemFound;

    ++buf->cur;
  }

  // Buffer is full
  return NULL;

l_itemFound:
  // Inactive item has been found!
  // Make it active and return it's location
  // in the buffer
  buf->active[buf->cur] = true;
  return buf->buf() + buf->cur++*buf->itemSize;
}

void FreeBufferItem(buffer_t *buf, void *item) {
  // Get item index
  const uptr ind = ((u8*)item - buf->buf()) / buf->itemSize;

  // This item isn't active anymore
  buf->active[ind] = false;
}
