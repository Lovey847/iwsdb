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
 * src/loveylib/endian.h:
 *  Endianness utilities
 *
 ************************************************************/

#ifndef _LOVEYLIB_ENDIAN_H
#define _LOVEYLIB_ENDIAN_H

#include "loveylib/types.h"
#include "loveylib_config.h"

// Compiler-specific intrinsics
#if defined(LOVEYLIB_GNU)

#define T_GNU_INTRIN(...) __VA_ARGS__
#define T_MSVC_INTRIN(...)
#define T_NO_INTRIN(...)

#elif defined(LOVEYLIB_MSVC)

// CRT (_byteswap_*)
#include <stdlib.h>

#define T_GNU_INTRIN(...)
#define T_MSVC_INTRIN(...) __VA_ARGS__
#define T_NO_INTRIN(...)

#else // No intrinsics

#define T_GNU_INTRIN(...)
#define T_MSVC_INTRIN(...)
#define T_NO_INTRIN(...) __VA_ARGS__

#endif

// Constant expression byte swapping macros
// If you're converting a value at runtime, you should
// use one of the byte swapping routines beluw.
//
// NOTE: Just because this is a 16-bit swapping routine,
//       doesn't mean it can't be given a 32-bit argument,
//       64-bit argument, etc.
#define CBYTE_SWAP16(x) ((((u16)(x)<<8)&0xff00) | (((u16)(x)>>8)&0x00ff))

#define CBYTE_SWAP32(x) ((((u32)(x) >> 24)&0x000000ff) |  \
                         (((u32)(x) >>  8)&0x0000ff00) |  \
                         (((u32)(x) <<  8)&0x00ff0000) |  \
                         (((u32)(x) << 24)&0xff000000))

#define CBYTE_SWAP64(x)                              \
  ((((u64)(x) >> U64(56))&U64(0x00000000000000ff)) | \
   (((u64)(x) >> U64(40))&U64(0x000000000000ff00)) | \
   (((u64)(x) >> U64(24))&U64(0x0000000000ff0000)) | \
   (((u64)(x) >> U64( 8))&U64(0x00000000ff000000)) | \
   (((u64)(x) << U64( 8))&U64(0x000000ff00000000)) | \
   (((u64)(x) << U64(24))&U64(0x0000ff0000000000)) | \
   (((u64)(x) << U64(40))&U64(0x00ff000000000000)) | \
   (((u64)(x) << U64(56))&U64(0xff00000000000000)))

// Runtime byte swapping routines
static inline u16 ByteSwap16(u16 v) {
  T_GNU_INTRIN (
    return __builtin_bswap16(v);
  )

  T_MSVC_INTRIN (
    return _byteswap_ushort(v);
  )

  T_NO_INTRIN (
    // Doesn't use CBYTE_SWAP16 because I don't wanna cast
    return (v<<8) | (v>>8);
  )
}

static inline u32 ByteSwap32(u32 v) {
  T_GNU_INTRIN (
    return __builtin_bswap32(v);
  )

  T_MSVC_INTRIN (
    return _byteswap_ulong(v);
  )

  T_NO_INTRIN (
    return (((v >> 24)&0x000000ff) |
            ((v >>  8)&0x0000ff00) |
            ((v <<  8)&0x00ff0000) |
            ((v << 24)&0xff000000));
  )
}

static inline u64 ByteSwap64(u64 v) {
  T_GNU_INTRIN (
    return __builtin_bswap64(v);
  )

  T_MSVC_INTRIN (
    return _byteswap_uint64(v);
  )

  T_NO_INTRIN (
    return (((v >> U64(56))&U64(0x00000000000000ff)) | \
            ((v >> U64(40))&U64(0x000000000000ff00)) | \
            ((v >> U64(24))&U64(0x0000000000ff0000)) | \
            ((v >> U64( 8))&U64(0x00000000ff000000)) | \
            ((v << U64( 8))&U64(0x000000ff00000000)) | \
            ((v << U64(24))&U64(0x0000ff0000000000)) | \
            ((v << U64(40))&U64(0x00ff000000000000)) | \
            ((v << U64(56))&U64(0xff00000000000000)));
  )
}

// Endian-dependent swapping
//
// Little endian machines: Swap big-endian vluaes
// Big endian machines: Swap little-endian values

#if defined(LOVEYLIB_LITTLE)

#define CLITTLE_ENDIAN16(x) (x)
#define CLITTLE_ENDIAN32(x) (x)
#define CLITTLE_ENDIAN64(x) (x)

#define CBIG_ENDIAN16(x) CBYTE_SWAP16(x)
#define CBIG_ENDIAN32(x) CBYTE_SWAP32(x)
#define CBIG_ENDIAN64(x) CBYTE_SWAP64(x)

static inline u16 LittleEndian16(u16 v) {return v;}
static inline u32 LittleEndian32(u32 v) {return v;}
static inline u64 LittleEndian64(u64 v) {return v;}

static inline u16 BigEndian16(u16 v) {return ByteSwap16(v);}
static inline u32 BigEndian32(u32 v) {return ByteSwap32(v);}
static inline u64 BigEndian64(u64 v) {return ByteSwap64(v);}

#elif defined(LOVEYLIB_BIG)

#define CLITTLE_ENDIAN16(x) CBYTE_SWAP16(x)
#define CLITTLE_ENDIAN32(x) CBYTE_SWAP32(x)
#define CLITTLE_ENDIAN64(x) CBYTE_SWAP64(x)

#define CBIG_ENDIAN16(x) (x)
#define CBIG_ENDIAN32(x) (x)
#define CBIG_ENDIAN64(x) (x)

static inline u16 LittleEndian16(u16 v) {return ByteSwap16(v);}
static inline u32 LittleEndian32(u32 v) {return ByteSwap32(v);}
static inline u64 LittleEndian64(u64 v) {return ByteSwap64(v);}

static inline u16 BigEndian16(u16 v) {return v;}
static inline u32 BigEndian32(u32 v) {return v;}
static inline u64 BigEndian64(u64 v) {return v;}

#else //defined(LOVEYLIB_BIG)
#error Unknown endianness!
#endif

// Get rid of temporary defines
#undef T_GNU_INTRIN
#undef T_NO_INTRIN

#endif //_LOVEYLIB_ENDIAN_H
