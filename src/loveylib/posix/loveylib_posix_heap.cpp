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
 * src/loveylib/posix/loveylib_posix_heap.cpp:
 *  Platform-dependent memory handling utilities
 *
 ************************************************************/

#include "loveylib/types.h"
#include "loveylib/heap.h"
#include "loveylib/utils.h"
#include "loveylib/assert.h"

#include <sys/mman.h>
#include <unistd.h>

// Private information stored at the start of the heap
struct heap_info_t {
  // Heap length
  uptr size;
};

// Get heap info from the heap
static heap_info_t *GetHeapInfo(heap_t h) {
  ASSERT(h);

  return (heap_info_t*)((u8*)h - GetPageSize());
}

uptr GetPageSize() {
  return sysconf(_SC_PAGESIZE);
}

heap_t InitHeap(uptr size) {
  ASSERT(size);

  const uptr pageSize = GetPageSize();

  // Fail if the page size isn't a power of 2
  if ((pageSize-1) & pageSize) return NULL;

  // Align size to the page size
  size = AlignUpPow2(size, pageSize);

  // Fail if the page size isn't big enough to hold heap info
  if (pageSize < sizeof(heap_info_t)) return NULL;

  // Use the first page of the heap to store specific information
  size += pageSize;

  // Allocate heap
  heap_info_t *info = (heap_info_t*)mmap(NULL, size,
                                     PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS,
                                     -1, 0);
  if (info == (heap_info_t*)MAP_FAILED) return NULL;

  // Setup heap info
  info->size = size;

  // Return remaining heap
  return (u8*)info + pageSize;
}

void DestroyHeap(heap_t heap) {
  ASSERT(heap);

  // This is the block we allocated
  heap_info_t *info = GetHeapInfo(heap);

  // Unmap heap, with heap size
  munmap(info, info->size);
}
