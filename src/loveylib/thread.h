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
 * src/loveylib/thread.h:
 *  Multi-threading interface
 *
 ************************************************************/

#ifndef _LOVEYLIB_THREAD_H
#define _LOVEYLIB_THREAD_H

#include "loveylib/types.h"
#include "loveylib/timer.h"

/*
 * THREAD LIFETIME
 *
 * Before a thread is created, it's uninitialized.
 *
 * When uninitialized, the thread is considered to not exist
 *
 * A thread shouldn't be referenced if it doesn't exist
 *
 * After CreateThread, the thread is running in it's entry point
 *
 * After the thread's entry point function returns, it's done
 *
 * In order to destroy a done thread, you must call DestroyThread on it
 *
 * When you destroy a done thread, it's put back into an uninitialized state,
 * doesn't exist, and shouldn't be referenced
 */

// Thread
struct alignas(16) thread_t {
  u8 privateData[64];
};

// Thread mutex
struct alignas(16) mutex_t {
  u8 privateData[48];
};

// Semaphore
struct alignas(16) semaphore_t {
  u8 privateData[32];
};

// Thread entry point
// The thread closes when this function
// returns
typedef void (*thread_entry_point_t)(void */*data*/);

// Create new thread
// out must be an uninitialized thread
// data is passed through to entry point
// Returns false on failure
bfast CreateThread(thread_t *out, thread_entry_point_t entry, void *data);

// Check if a thread is running
// Returns true if the thread is running, false if it isn't
bfast ThreadRunning(thread_t *thread);

// Check if this is the calling thread
// Returns true if this is the calling thread,
// false if this is another thread
bfast IsCallingThread(thread_t *thread);

// Wait for thread to be done
// NOTE: The thread isn't implicitly destroyed by this call
// Returns false if the thread is done
bfast WaitThread(thread_t *thread);

// Destroy thread that is done
// Returns false if the thread isn't done
bfast DestroyThread(thread_t *thread);

// Create mutex
bfast CreateMutex(mutex_t *mutex);

// Lock mutex, blocks until mutex is available
// timeout is specified in seconds
// If timeout is 0, there is no timeout
// If lock times out, the return value is false
// Otherwise, the return value is true
bfast LockMutex(mutex_t *mutex, u32 timeout = 0);

// Unlock mutex, return false if calling thread
// doesn't own mutex
bfast UnlockMutex(mutex_t *mutex);

// Lock mutex, unless mutex is already locked by another thread
// Returns true if mutex was locked, false if it wasn't
bfast LockMutexIfAvailable(mutex_t *mutex);

// Destroy mutex
// MAKE SURE MUTEX ISN'T LOCKED BY ANY THREAD
void DestroyMutex(mutex_t *mutex);

// Create semaphore
// Returns false on failure
bfast CreateSema(semaphore_t *out);

// Wait on semaphore
// timeout is the same as it is in LockMutex
// Returns false if the function times out
bfast WaitSema(semaphore_t *sema, u32 timeout = 0);

// Signal semaphore
bfast SignalSema(semaphore_t *sema);

// Destroy semaphore
// MAKE SURE NO THREADS ARE WAITING ON THIS SEMA
void DestroySema(semaphore_t *sema);

#endif //_LOVEYLIB_THREAD_H
