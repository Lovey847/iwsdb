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
 * src/loveylib/posix/loveylib_posix_thread.cpp
 *  POSIX multi-threading implementation
 *
 ************************************************************/

// sem_timedwait/clock_gettime
#define _POSIX_C_SOURCE 200112L

#include "loveylib/types.h"
#include "loveylib/thread.h"
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

#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <atomic>

// Thread state
enum thread_state_e : ufast {
  THREAD_RUNNING, // The thread is currently running
  THREAD_FINISHED, // The thread is done running
  THREAD_JOINED // The thread has been joined with
};
typedef ufast thread_state_t;

// Private thread, mutex, and semaphore data
struct posix_thread_t {
  pthread_t tid;
  void *data;
  thread_entry_point_t entry;

  std::atomic<thread_state_t> state;
};
static_assert(sizeof(posix_thread_t) <= sizeof(thread_t), "");

struct posix_mutex_t {
  pthread_mutex_t m;
};
static_assert(sizeof(posix_mutex_t) <= sizeof(mutex_t), "");

struct posix_semaphore_t {
  sem_t s;
};
static_assert(sizeof(posix_semaphore_t) <= sizeof(semaphore_t), "");

// POSIX thread entry point wrapper
static void *EntryPoint(void *thread) {
  posix_thread_t *t = (posix_thread_t*)thread;

  // Run actual entry point
  t->entry(t->data);

  // Thread isn't open anymore
  t->state.store(THREAD_FINISHED, std::memory_order_relaxed);

  // Exit thread
  pthread_exit(NULL);
}

bfast CreateThread(thread_t *out, thread_entry_point_t entry, void *data) {
  posix_thread_t *t = (posix_thread_t*)out;

  t->entry = entry;
  t->data = data;
  t->state.store(THREAD_RUNNING, std::memory_order_relaxed);
  const int ret = pthread_create(&t->tid, NULL, EntryPoint, t);
  if (ret != 0) {
    return false;
  }

  return true;
}

bfast ThreadRunning(thread_t *thread) {
  posix_thread_t *t = (posix_thread_t*)thread;

  return t->state.load(std::memory_order_relaxed) == THREAD_RUNNING;
}

bfast IsCallingThread(thread_t *thread) {
  posix_thread_t *t = (posix_thread_t*)thread;
  return pthread_self() == t->tid;
}

bfast WaitThread(thread_t *thread) {
  posix_thread_t *t = (posix_thread_t*)thread;

  // If the thread isn't open, return false
  if (t->state.load(std::memory_order_relaxed) != THREAD_RUNNING) return false;

  // Join with the thread
  pthread_join(t->tid, NULL);
  t->state.store(THREAD_JOINED, std::memory_order_relaxed);

  return true;
}

bfast DestroyThread(thread_t *thread) {
  posix_thread_t *t = (posix_thread_t*)thread;

  thread_state_t s = t->state.load(std::memory_order_relaxed);
  // If the thread is finished or joined, return true
  if (s >= THREAD_FINISHED) {
    // If the thread is finished, join it
    if (s == THREAD_FINISHED) pthread_join(t->tid, NULL);
    return true;
  }

  // If the thread isn't finished nor joined (still running), return false
  return false;
}

bfast CreateMutex(mutex_t *mutex) {
  posix_mutex_t *m = (posix_mutex_t*)mutex;

  if (pthread_mutex_init(&m->m, NULL) != 0) return false;
  return true;
}

bfast LockMutex(mutex_t *mutex, u32 timeout) {
  posix_mutex_t *m = (posix_mutex_t*)mutex;
  struct timespec t;

  if (timeout) {
    clock_gettime(CLOCK_REALTIME, &t);
    t.tv_sec += timeout;
    if (pthread_mutex_timedlock(&m->m, &t) != 0) return false;
  } else if (pthread_mutex_lock(&m->m) != 0) return false;

  return true;
}

bfast UnlockMutex(mutex_t *mutex) {
  posix_mutex_t *m = (posix_mutex_t*)mutex;

  if (pthread_mutex_unlock(&m->m) != 0) return false;

  return true;
}

bfast LockMutexIfAvailable(mutex_t *mutex) {
  posix_mutex_t *m = (posix_mutex_t*)mutex;

  bfast ret = pthread_mutex_trylock(&m->m) == 0;
  return ret;
}

void DestroyMutex(mutex_t *mutex) {
  posix_mutex_t *m = (posix_mutex_t*)mutex;

  pthread_mutex_destroy(&m->m);
}

bfast CreateSema(semaphore_t *out) {
  posix_semaphore_t *s = (posix_semaphore_t*)out;

  if (sem_init(&s->s, 0, 0) < 0) return false;
  return true;
}

bfast WaitSema(semaphore_t *sema, u32 timeout) {
  posix_semaphore_t *s = (posix_semaphore_t*)sema;
  struct timespec t;

  if (timeout) {
    clock_gettime(CLOCK_REALTIME, &t);
    t.tv_sec += timeout;
    if (sem_timedwait(&s->s, &t) < 0) return false;
  } else if (sem_wait(&s->s) < 0) return false;

  return true;
}

bfast SignalSema(semaphore_t *sema) {
  posix_semaphore_t *s = (posix_semaphore_t*)sema;

  if (sem_post(&s->s) < 0) return false;
  return true;
}

void DestroySema(semaphore_t *sema) {
  posix_semaphore_t *s = (posix_semaphore_t*)sema;
  sem_destroy(&s->s);
}

#endif //LOVEYLIB_THREADS
