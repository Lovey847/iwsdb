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
 * src/loveylib/win32/loveylib_win32_thread.cpp
 *  Win32 multi-threading implementation
 *
 ************************************************************/

#include "loveylib/types.h"
#include "loveylib/thread.h"
#include "loveylib/assert.h"
#include "loveylib/utils.h"
#include "loveylib_config.h"

// If we don't have threads, make dummy wrappers
#ifndef LOVEYLIB_THREADS

bfast CreateThread(thread_t*, thread_entry_point_t, void*) {return false;}
bfast ThreadRunning(thread_t*) {return false;}
bfast IsCallingThread(thread_t*) {return false;}
bfast WaitThread(thread_t*) {return false;}
bfast DestroyThread(thread_t*) {return false;}
bfast CreateMutex(mutex_t*) {return false;}
bfast LockMutex(mutex_t*, u32) {return false;}
bfast UnlockMutex(mutex_t*) {return false;}
bfast LockMutexIfAvailable(mutex_t*) {return false;}
void DestroyMutex(mutex_t*) {}
bfast CreateSema(semaphore_t*) {return false;}
bfast WaitSema(semaphore_t*, u32) {return false;}
bfast SignalSema(semaphore_t*) {return false;}
void DestroySema(semaphore_t*) {}

#else //LOVEYLIB_THREADS

#include "loveylib/win32/loveylib_windows.h"

// Private thread, mutex, and semaphore data
static constexpr const u32 THREAD_MAGIC = 0x334db9a;
struct win32_thread_t {
  win32::handle_t handle; // Thread handle
  void *data;
  thread_entry_point_t entry;
  IN_DEBUG(u32 magic); // == THREAD_MAGIC
};
static_assert(sizeof(win32_thread_t) <= sizeof(thread_t), "");

struct win32_mutex_t {
  win32::handle_t handle; // Mutex handle
};
static_assert(sizeof(win32_mutex_t) <= sizeof(mutex_t), "");

struct win32_semaphore_t {
  win32::handle_t handle; // Semaphore handle
};
static_assert(sizeof(win32_semaphore_t) <= sizeof(semaphore_t), "");

// Win32 thread entry point wrapper
static u32 WIN32_WINAPI EntryPoint(void *param) {
  win32_thread_t *t = (win32_thread_t*)param;

  t->entry(t->data);

  win32::ExitThread(0);
  return 0;
}

bfast CreateThread(thread_t *out, thread_entry_point_t entry, void *data) {
  win32_thread_t *t = (win32_thread_t*)out;

  t->entry = entry;
  t->data = data;
  t->handle = win32::CreateThread(NULL, 0, EntryPoint, t, 0, NULL);
  if (!t->handle) return false;

  IN_DEBUG(t->magic = THREAD_MAGIC);

  return true;
}

bfast ThreadRunning(thread_t *thread) {
  win32_thread_t *t = (win32_thread_t*)thread;
  ASSERT(t->magic == THREAD_MAGIC);

  u32 exitCode;
  if (!win32::GetExitCodeThread(t->handle, &exitCode)) return false;

  return exitCode == win32::STILL_ACTIVE;
}

bfast IsCallingThread(thread_t *thread) {
  win32_thread_t *t = (win32_thread_t*)thread;
  ASSERT(t->magic == THREAD_MAGIC);
  return win32::GetCurrentThreadId() == win32::GetThreadId(t->handle);
}

bfast WaitThread(thread_t *thread) {
  win32_thread_t *t = (win32_thread_t*)thread;
  ASSERT(t->magic == THREAD_MAGIC);

  if (!ThreadRunning(thread)) return false;
  return win32::WaitForSingleObject(t->handle, win32::INFINITE) != win32::WAIT_FAILED;
}

bfast DestroyThread(thread_t *thread) {
  win32_thread_t *t = (win32_thread_t*)thread;
  ASSERT(t->magic == THREAD_MAGIC);

  if (ThreadRunning(thread)) return false;

  IN_DEBUG(t->magic = 0);
  return win32::CloseHandle(t->handle);
}

bfast CreateMutex(mutex_t *mutex) {
  win32_mutex_t *m = (win32_mutex_t*)mutex;

  m->handle = win32::CreateMutex(NULL, false, NULL);
  return m->handle != NULL;
}

bfast LockMutex(mutex_t *mutex, u32 timeout) {
  win32_mutex_t *m = (win32_mutex_t*)mutex;
  if (timeout == 0) timeout = win32::INFINITE;
  else timeout *= 1000;

  return win32::WaitForSingleObject(m->handle, timeout) == win32::WAIT_OBJECT_0;
}

bfast UnlockMutex(mutex_t *mutex) {
  win32_mutex_t *m = (win32_mutex_t*)mutex;

  return win32::ReleaseMutex(m->handle);
}

bfast LockMutexIfAvailable(mutex_t *mutex) {
  win32_mutex_t *m = (win32_mutex_t*)mutex;

  return win32::WaitForSingleObject(m->handle, 0) == win32::WAIT_OBJECT_0;
}

void DestroyMutex(mutex_t *mutex) {
  win32_mutex_t *m = (win32_mutex_t*)mutex;

  win32::CloseHandle(m->handle);
}

bfast CreateSema(semaphore_t *out) {
  win32_semaphore_t *s = (win32_semaphore_t*)out;

  s->handle = win32::CreateSemaphore(NULL, 0, 256, NULL);
  return s->handle != NULL;
}

bfast WaitSema(semaphore_t *sema, u32 timeout) {
  win32_semaphore_t *s = (win32_semaphore_t*)sema;
  if (!timeout) timeout = win32::INFINITE;
  else timeout *= 1000;

  return win32::WaitForSingleObject(s->handle, timeout) == win32::WAIT_OBJECT_0;
}

bfast SignalSema(semaphore_t *sema) {
  win32_semaphore_t *s = (win32_semaphore_t*)sema;
  return win32::ReleaseSemaphore(s->handle, 1, NULL);
}

void DestroySema(semaphore_t *sema) {
  win32_semaphore_t *s = (win32_semaphore_t*)sema;
  win32::CloseHandle(s->handle);
}

#endif //LOVEYLIB_THREADS
