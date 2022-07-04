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
 * src/loveylib/win32/loveylib_win32_heap.cpp:
 *  Win32 heap allocation
 *
 ************************************************************/

#include "loveylib/types.h"
#include "loveylib/assert.h"
#include "loveylib/utils.h"
#include "loveylib/heap.h"

#include "loveylib/win32/loveylib_windows.h"

uptr GetPageSize() {
  win32::system_info_t info;
  win32::GetSystemInfo(&info);

  return info.pageSize;
}

heap_t InitHeap(uptr size) {
  ASSERT(size);

  const uptr pageSize = GetPageSize();

  // Fail if the page size isn't a power of 2
  if ((pageSize-1) & pageSize) return NULL;

  // Align size up to the page size
  size = AlignUpPow2(size, pageSize);

  // Allocate heap
  heap_t ret = win32::VirtualAlloc(NULL, size,
                                   win32::MEM_RESERVE|win32::MEM_COMMIT,
                                   win32::PAGE_READWRITE);

  // Return heap, NULL if allocation failed
  return ret;
}

void DestroyHeap(heap_t heap) {
  ASSERT(heap);

  // Free heap
  win32::VirtualFree(heap, 0, win32::MEM_RELEASE);
}
