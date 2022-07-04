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
 * src/loveylib/mem.h:
 *  Memory manager
 *
 ************************************************************/

#ifndef _LOVEYLIB_MEM_H
#define _LOVEYLIB_MEM_H

#include "loveylib/types.h"
#include "loveylib/utils.h"

// Return type for functions that return allocated memory
// that require the caller to free them
template<typename T>
struct alloced_ret_t {
  // Once claimed, you are responsible for freeing it
  T claimMemory;

  constexpr alloced_ret_t(T init) : claimMemory(init) {}
};

///////////////////
// Memory arena

// Opaque memory arena
struct mem_arena_t;

// Arena allocation alignment
static constexpr const uptr ARENA_ALIGNMENT = 64;

// Fixed-size memory arena container
template<uptr size>
struct alignas(ARENA_ALIGNMENT) mem_arena_container_t {
  u8 data[size];

  constexpr operator const mem_arena_t*() const {return (mem_arena_t*)data;}
  inline operator mem_arena_t*() {return (mem_arena_t*)data;}
};

// Arena allocation flags
enum arena_alloc_flags_e : ufast {
  // No allocation flags yet
};
typedef ufast arena_alloc_flags_t;

// Memory arena initialization
bfast InitMemoryArena(mem_arena_t *out, uptr size);

template<uptr size>
static inline bfast InitMemoryArena(mem_arena_container_t<size> *out) {
  return InitMemoryArena((mem_arena_t*)out, size);
}

// Allocate memory from memory arena
void *Alloc(mem_arena_t *a, uptr size,
            const char *name = "", arena_alloc_flags_t flags = 0);

// Free memory in arena
void Free(mem_arena_t *a, void *addr);

#endif //_LOVEYLIB_MEM_H
