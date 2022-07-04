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
 * src/loveylib/types.h:
 *  This file defines the integral types of loveylib.
 *  They're also just useful types to have in general.
 *
 ************************************************************/

#ifndef _LOVEYLIB_TYPES_H
#define _LOVEYLIB_TYPES_H

// NOTE: These includes might be removed/made optional in the future.
#include <cstddef>
#include <cstdint>
#include <cfloat>

////////////////////////////////
// Fixed-size integer types
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/////////////////////////////////////
// Fixed-size floating-point types

// IEE754 single-precision floating point type
#if DBL_MANT_DIG == 24
typedef double f32;
#elif FLT_MANT_DIG == 24
typedef float f32;
#else
#error "Cannot find ieee754 single-precision floating point type!"
#endif

// IEE754 double-precision floating point type
#if LDBL_MANT_DIG == 53
typedef long double f64;
#elif DBL_MANT_DIG == 53
typedef double f64;
#else
#error "Cannot find ieee754 double-precision floating point type!"
#endif

// Make sure the size is correct
static_assert(sizeof(f32) == 4, "Invalid f32 size!");
static_assert(sizeof(f64) == 8, "Invalid f64 size!");

///////////////////////////
// Fastest integer types
typedef int_fast8_t ifast;
typedef uint_fast8_t ufast;

/////////////////////////////////
// Pointer size integer types
// Use intptr_t, if available
#if defined(INTPTR_MAX) && defined(UINTPTR_MAX)

typedef intptr_t iptr;
typedef uintptr_t uptr;

#else

typedef size_t iptr;
typedef ptrdiff_t uptr;

#endif

//////////////////
// Boolean types
typedef ufast bfast;
typedef u8 b8;
typedef u16 b16;
typedef u32 b32;
typedef u64 b64;
typedef uptr bptr;

//////////////////////////
// Type suffixes

// i64/u64
#if ULONG_MAX == 4294967295

#define I64(_num) _num ## ll
#define U64(_num) _num ## ull

#else

#define I64(_num) _num ## l
#define U64(_num) _num ## ul

#endif // ULONG_MAX == 4294967295

#endif //_LOVEYLIB_TYPES_H
