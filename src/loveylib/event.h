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
 * src/loveylib/event.h:
 *  Event information
 *
 ************************************************************/

#ifndef _LOVEYLIB_EVENT_H
#define _LOVEYLIB_EVENT_H

#include "loveylib/types.h"

// Event type
enum event_type_e : ufast {
  KEY_EVENT = 0, // Keyboard event
  CLOSE_EVENT, // App closed event

  EVENT_COUNT
};
typedef ufast event_type_t;

// Keyboard event flags
enum key_flags_e : ufast {
  KEY_RELEASED_FLAG = 0,
  KEY_AUTOREPEAT_FLAG,

  KEY_RELEASED_BIT = 1<<KEY_RELEASED_FLAG,
  KEY_AUTOREPEAT_BIT = 1<<KEY_AUTOREPEAT_FLAG,
};
typedef ufast key_flags_t;

// Keyboard key codes
#include "loveylib/loveylib_key_codes.h"

// Keyboard event
struct key_event_t {
  // Key in question
  key_code_t code;

  // Flags
  key_flags_t flags;
};

// Event
struct event_t {
  union {
    key_event_t key;
  };

  event_type_t type;
};

#endif //_LOVEYLIB_EVENT_H
