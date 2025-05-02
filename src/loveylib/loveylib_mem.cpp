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
 * src/loveylib/loveylib_mem.cpp:
 *  Memory manager
 *
 ************************************************************/

#include "loveylib/types.h"
#include "loveylib/mem.h"
#include "loveylib/utils.h"
#include "loveylib/heap.h"
#include "loveylib/assert.h"

#include <cstring>

//////////////////////////////////////
// Memory arena

/*
 * Memory arena layout:
 *
 * Chunk offset         Memory area description
 *
 * 0                    ==================================
 *                      |
 *                      | Memory arena header
 *                      |
 * sizeof(mem_arena_t)  |=================================
 *                      |
 *                      | Block area
 *                      |
 *                      |
 *                      |
 *                      |
 *                      |
 *                      |
 *                      |
 *                      |
 *                      |
 *                      |
 * Chunk size           ==================================
 */

// Cache entry forward declaration
struct free_arena_block_t;

// Memory arena block header
static constexpr const uptr ARENA_BLOCK_HDR_SIZE = ARENA_ALIGNMENT;
struct arena_block_hdr_t {
  arena_block_hdr_t *prev, *next; // Previous & next block
  uptr size; // Block size
  free_arena_block_t *cache; // Pointer to cache entry, NULL if not in the cache

  arena_alloc_flags_t flags; // Block flags
  bfast active; // Whether this block is active or not

  // Block name (remainder of block header size)
  inline char *name() {return (char*)(this+1);}
  inline const char *name() const {return (const char*)(this + 1);}

  // Block data
  inline void *data() {return (u8*)this + ARENA_BLOCK_HDR_SIZE;}
};

// At least a NULL terminator for a name must be stored in a block
static_assert(ARENA_BLOCK_HDR_SIZE > sizeof(arena_block_hdr_t), "");

// Size of free block cache
// A bigger value uses more memory, but
// msy make finding free blocks faster
static constexpr const uptr MAX_FREE_BLOCKS = 7;

// Free block descriptor
struct free_arena_block_t {
  arena_block_hdr_t *addr;
  uptr size;
};

// Memory arena header
struct alignas(ARENA_ALIGNMENT) mem_arena_t {
  // Free block cache, to potentially speed
  // up allocation
  free_arena_block_t freeBlocks[MAX_FREE_BLOCKS];

  // Current free block pointer
  free_arena_block_t *curFree;

  // First block
  inline arena_block_hdr_t *blockList() {
    return (arena_block_hdr_t*)(this+1);
  }
};

// Blocks after header must be aligned
static_assert((sizeof(mem_arena_t) & (ARENA_ALIGNMENT-1)) == 0, "");

// Fill out free arena block
static inline void FillFreeBlock(free_arena_block_t *ent, arena_block_hdr_t *blk) {
  ASSERT(!blk->active);
  ASSERT(ent);

  ent->addr = blk;
  ent->size = blk->size;
  blk->cache = ent;
}

// Validate cache entry, if cache entry
// is invalid it'll be nulled out
#ifdef UNUSED
static inline bptr ValidateCache(free_arena_block_t *free) {
  ASSERT(free->addr);

  if (free->addr->cache != free) free->addr->cache = NULL;

  return (bptr)free->addr->cache;
}
#endif  //UNUSED

// Same thing, but for block
static inline bptr ValidateCache(arena_block_hdr_t *blk) {
  if (blk->cache && blk->cache->addr != blk) blk->cache = NULL;

  return (bptr)blk->cache;
}

// Add block to free block cache
static void AddFreeBlock(mem_arena_t *a, arena_block_hdr_t *blk) {
  ASSERT(!blk->cache);

  FillFreeBlock(a->curFree++, blk);

  // Wrap around
  if (a->curFree >= ArrayEnd(a->freeBlocks))
    a->curFree = a->freeBlocks;
}

// Remove block from free block cache
static void RemoveFreeBlock(mem_arena_t *a, arena_block_hdr_t *blk) {
  // Make sure cache entry is valid before removing it
  if (ValidateCache(blk)) {
    // Cache entries with size 0 will be ignored
    blk->cache->size = 0;

    // If curFree isn't already on a free position,
    // put it here
    if (a->curFree->size != 0) a->curFree = blk->cache;

    // Block isn't in cache anymore
    blk->cache = NULL;
  }
}

// Replace a block from free block cache
static void ReplaceFreeBlock(arena_block_hdr_t *oldBlk,
                             arena_block_hdr_t *newBlk)
{
  ASSERT(!newBlk->active);
  ASSERT(!newBlk->cache);

  // Make sure cache entry is valid
  if (!ValidateCache(oldBlk)) return;

  FillFreeBlock(oldBlk->cache, newBlk);
  oldBlk->cache = NULL;
}

bfast InitMemoryArena(mem_arena_t *hdr, uptr size) {
  // Align size
  size = AlignDownMask(size, ARENA_ALIGNMENT-1);

  // If this chunk isn't big enough for a memory arena, fail
  if (size <= sizeof(mem_arena_t) + ARENA_BLOCK_HDR_SIZE)
    return false;

  // Setup arena
  memset(hdr, 0, sizeof(mem_arena_t));

  arena_block_hdr_t *blockHdr = hdr->blockList();
  memset(blockHdr, 0, sizeof(arena_block_hdr_t));

  blockHdr->size = size - (sizeof(mem_arena_t) +
                           ARENA_BLOCK_HDR_SIZE);
  blockHdr->cache = hdr->freeBlocks;

  // Setup free blocks
  hdr->freeBlocks[0].addr = blockHdr;
  hdr->freeBlocks[0].size = blockHdr->size;
  hdr->curFree = &hdr->freeBlocks[1];

  // This was a success
  return true;
}

// Find a free block
static arena_block_hdr_t *FindFreeBlock(mem_arena_t *arena, uptr size) {
  // First, search through cache
  for (free_arena_block_t *i = arena->freeBlocks;
       i != ArrayEnd(arena->freeBlocks); ++i)
  {
    if (i->size >= size) return i->addr;
  }

  // Nothing found in the cache, looks like we gotta do a slow search
  for (arena_block_hdr_t *i = arena->blockList();
       i; i = i->next)
  {
    if (!i->active && (i->size >= size)) return i;
  }

  // No block was found!
  return NULL;
}

void *Alloc(mem_arena_t *arena, uptr size,
            const char *name, arena_alloc_flags_t flags)
{
  // TODO: Ignoring name for now
  (void)name;

  // Find free block
  arena_block_hdr_t *free = FindFreeBlock(arena, size);
  if (!free) return NULL;

  // Setup block
  free->active = true;
  free->flags = flags;

  // If user-supplied size and block size differ by
  // 2 or more block headers, add free block
  if (free->size - size >= ARENA_BLOCK_HDR_SIZE*2) {
    size = AlignUpPow2(size, ARENA_ALIGNMENT);
    arena_block_hdr_t *next = (arena_block_hdr_t*)((u8*)free->data() + size);

    // Setup next block
    memset(next, 0, ARENA_BLOCK_HDR_SIZE);

    next->prev = free;
    next->next = free->next;
    next->size = free->size - size - ARENA_BLOCK_HDR_SIZE;
    next->cache = NULL;

    // Set next block and new size
    free->size = size;
    free->next = next;

    // Replace this block in the cache
    ReplaceFreeBlock(free, free->next);
  } else {
    // Remove this block from the cache
    RemoveFreeBlock(arena, free);
  }

  return free->data();
}

// Get arena block header from address
static inline arena_block_hdr_t *GetArenaBlock(void *addr) {
  return (arena_block_hdr_t*)((u8*)addr - ARENA_BLOCK_HDR_SIZE);
}

// Concatenate 2 free blocks (a < b)
static arena_block_hdr_t *ConcatBlocks(mem_arena_t *arena, arena_block_hdr_t *a,
                                       arena_block_hdr_t *b)
{
  ASSERT(!a->active);
  ASSERT(!b->active);
  ASSERT(a->next == b);
  ASSERT(b->prev == a);

  // a consumes b's size
  a->size += b->size + ARENA_BLOCK_HDR_SIZE;

  // Since b is gone, adjust linked list
  a->next = b->next;
  if (a->next) a->next->prev = a;

  // Update a's cache entry
  if (ValidateCache(a))
    a->cache->size = a->size;

  // Remove b from the cache
  RemoveFreeBlock(arena, b);

  return a;
}

void Free(mem_arena_t *arena, void *addr) {
  // Get block from void*
  arena_block_hdr_t *blk = GetArenaBlock(addr);

  // This block is now free
  blk->active = false;
  AddFreeBlock(arena, blk);

  // While previous block is free, concatenate
  if (blk->prev && !blk->prev->active)
    blk = ConcatBlocks(arena, blk->prev, blk);

  // While next block is free, concatenate
  if (blk->next && !blk->next->active)
    blk = ConcatBlocks(arena, blk, blk->next);
}
