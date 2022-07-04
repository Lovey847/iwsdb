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
 * src/loveylib/assert.h:
 *  Break if a condition isn't met
 *
 ************************************************************/

#ifndef _LOVEYLIB_ASSERT_H
#define _LOVEYLIB_ASSERT_H

#include "loveylib/types.h"
#include "loveylib_config.h"

// Don't DEBUG_BREAK and ASSERT in release
#ifdef NDEBUG

#define DEBUG_BREAK 0
#define ASSERT(condition)

#else //NDEBUG

// DEBUG_BREAK is __builtin_trap with GNU extensions
#if defined(LOVEYLIB_GNU)

#define DEBUG_BREAK __builtin_trap()

// DEBUG_BREAK is *(u8*)NULL = 0 if nothing else is available
#else //defined(LOVEYLIB_GNU)

#define DEBUG_BREAK (*(u8*)NULL = 0)

#endif //defined(LOVEYLIB_GNU)

// ASSERT is meant to be called on it's own
#define ASSERT(condition) if (!(condition)) DEBUG_BREAK

#endif //NDEBUG

#endif //_LOVEYLIB_ASSERT_H
