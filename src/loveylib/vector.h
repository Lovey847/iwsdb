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
 * src/loveylib/vector.h:
 *  This file contains vector routines that can be
 *  optimized with various forms of SIMD
 *
 ************************************************************/

#ifndef _LOVEYLIB_VECTOR_H
#define _LOVEYLIB_VECTOR_H

#include "loveylib/types.h"
#include "loveylib_config.h"

// SIMD-specific intrinsics
// For now, SSE is only supported on little-endian machines
#if defined(LOVEYLIB_SSE) && defined(LOVEYLIB_LITTLE)

#include <xmmintrin.h>
#include <emmintrin.h>

#define T_SSE_INTRIN(...) __VA_ARGS__
#define T_NO_INTRIN(...)

#else // No SIMD

#define T_SSE_INTRIN(...)
#define T_NO_INTRIN(...) __VA_ARGS__

#endif

// Vector types

// Floating-point 4D vector
union alignas(16) vec4 {
  f32 v[4];
  i32 i[4];
  u32 u[4];
  u8 b[8];
  u64 h[2];

  T_SSE_INTRIN (
    __m128 p;
    __m128i ip;
  )
};

// Integer 4D vector
union alignas(16) ivec4 {
  i32 v[4];
  f32 f[4];
  u32 u[4];
  u8 b[8];
  u64 h[2];

  T_SSE_INTRIN (
    __m128i p;
    __m128 fp;
  )
};

// Vector data initializing types
// Can be loaded with LoadVec4
struct alignas(16) vec4_f_data_t {f32 x, y, z, w;};
struct alignas(16) vec4_i_data_t {i32 x, y, z, w;};
struct alignas(16) vec4_u_data_t {u32 x, y, z, w;};

// Vector setting
// ZeroVec4
static inline vec4 ZeroVec4() {
  vec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_setzero_ps();
  );

  T_NO_INTRIN (
    ret.h[0] = ret.h[1] = 0;
  );

  return ret;
}
static inline ivec4 ZeroIvec4() {
  ivec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_setzero_si128();
  );

  T_NO_INTRIN (
    ret.h[0] = ret.h[1] = 0;
  );

  return ret;
}

// LoadVec4 (data MUST be aligned to 16-byte boundary!)
static inline vec4 LoadVec4(const void *data) {
  vec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_load_ps((const f32*)data);
  );

  T_NO_INTRIN (
    ret.h[0] = ((u64*)data)[0];
    ret.h[1] = ((u64*)data)[1];
  );

  return ret;
}
static inline ivec4 LoadIvec4(const void *data) {
  ivec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_load_si128((const __m128i*)data);
  );

  T_NO_INTRIN (
    ret.h[0] = ((u64*)data)[0];
    ret.h[1] = ((u64*)data)[1];
  );

  return ret;
}

// CreateVec4
static inline vec4 CreateVec4(const vec4_f_data_t &v) {
  return LoadVec4(&v);
}
static inline ivec4 CreateIvec4(const vec4_i_data_t &v) {
  return LoadIvec4(&v);
}
static inline ivec4 CreateUvec4(const vec4_u_data_t &v) {
  return LoadIvec4(&v);
}

#define VEC4(x, y, z, w) CreateVec4({x, y, z, w})
#define VEC4_1(v) CreateVec4({v, v, v, v})
#define IVEC4_F(x, y, z, w) Ivec4Cast(CreateVec4({x, y, z, w}))
#define IVEC4_F_1(v) Ivec4Cast(CreateVec4({v, v, v, v}))

#define VEC4_I(x, y, z, w) Vec4Cast(CreateIvec4({x, y, z, w}))
#define VEC4_I_1(v) Vec4Cast(CreateIvec4({v, v, v, v}))
#define IVEC4(x, y, z, w) CreateIvec4({x, y, z, w})
#define IVEC4_1(v) CreateIvec4({v, v, v, v})

#define VEC4_U(x, y, z, w) VEC4_I((i32)(x), (i32)(y), (i32)(z), (i32)(w))
#define VEC4_U_1(v) VEC4_I_1((i32)(v))
#define IVEC4_U(x, y, z, w) IVEC4((i32)(x), (i32)(y), (i32)(z), (i32)(w))
#define IVEC4_U_1(v) IVEC4((i32)(v), (i32)(v), (i32)(v), (i32)(v))

// Vector routines

// Mask is 0xXYZW, X/Y/Z/W can be one of 4 values:
//  0: X
//  1: Y
//  2: Z
//  3: W
//
// Examples:
//  0x0123 = XYZW
//  0x0321 = XWZY
//  0x0022 = XXZZ
//  0x3313 = WWYW

// ShuffleVec4
template<u32 mask>
static inline vec4 ShuffleVec4(const vec4 &vec) {
  vec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_shuffle_ps(vec.p, vec.p, (((mask&0x3000)>>12) |
                                          ((mask&0x0300)>> 6) |
                                          ((mask&0x0030)    ) |
                                          ((mask&0x0003)<< 6)));
  );

  T_NO_INTRIN (
    ret.v[0] = vec.v[(mask>>12)    ];
    ret.v[1] = vec.v[(mask>> 8)&0x3];
    ret.v[2] = vec.v[(mask>> 4)&0x3];
    ret.v[3] = vec.v[(mask    )&0x3];
  );

  return ret;
}
template<u32 mask>
static inline ivec4 ShuffleVec4(const ivec4 &vec) {
  ivec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_shuffle_epi32(vec.p, (((mask&0x3000)>>12) |
                                      ((mask&0x0300)>> 6) |
                                      ((mask&0x0030)    ) |
                                      ((mask&0x0003)<< 6)));
  );

  T_NO_INTRIN (
    ret.v[0] = vec.v[(mask>>12)    ];
    ret.v[1] = vec.v[(mask>> 8)&0x3];
    ret.v[2] = vec.v[(mask>> 4)&0x3];
    ret.v[3] = vec.v[(mask    )&0x3];
  );

  return ret;
}

// operator+
static inline vec4 operator+(const vec4 &left, const vec4 &right) {
  vec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_add_ps(left.p, right.p);
  );

  T_NO_INTRIN (
    ret.v[0] = left.v[0] + right.v[0];
    ret.v[1] = left.v[1] + right.v[1];
    ret.v[2] = left.v[2] + right.v[2];
    ret.v[3] = left.v[3] + right.v[3];
  );

  return ret;
}
static inline ivec4 operator+(const ivec4 &left, const ivec4 &right) {
  ivec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_add_epi32(left.p, right.p);
  );

  T_NO_INTRIN (
    ret.v[0] = left.v[0] + right.v[0];
    ret.v[1] = left.v[1] + right.v[1];
    ret.v[2] = left.v[2] + right.v[2];
    ret.v[3] = left.v[3] + right.v[3];
  );

  return ret;
}

// operator-
static inline vec4 operator-(const vec4 &left, const vec4 &right) {
  vec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_sub_ps(left.p, right.p);
  );

  T_NO_INTRIN (
    ret.v[0] = left.v[0] - right.v[0];
    ret.v[1] = left.v[1] - right.v[1];
    ret.v[2] = left.v[2] - right.v[2];
    ret.v[3] = left.v[3] - right.v[3];
  );

  return ret;
}
static inline ivec4 operator-(const ivec4 &left, const ivec4 &right) {
  ivec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_sub_epi32(left.p, right.p);
  );

  T_NO_INTRIN (
    ret.v[0] = left.v[0] - right.v[0];
    ret.v[1] = left.v[1] - right.v[1];
    ret.v[2] = left.v[2] - right.v[2];
    ret.v[3] = left.v[3] - right.v[3];
  );

  return ret;
}

// operator*
static inline vec4 operator*(const vec4 &left, const vec4 &right) {
  vec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_mul_ps(left.p, right.p);
  );

  T_NO_INTRIN (
    ret.v[0] = left.v[0] * right.v[0];
    ret.v[1] = left.v[1] * right.v[1];
    ret.v[2] = left.v[2] * right.v[2];
    ret.v[3] = left.v[3] * right.v[3];
  );

  return ret;
}
static inline ivec4 operator*(const ivec4 &left, const ivec4 &right) {
  ivec4 ret;

  T_SSE_INTRIN (
    // Multiply 2 4D vectors into 2 2D vectors
    const __m128i left13Right13 = _mm_mul_epu32(left.p, right.p);
    const __m128i left24Right24 = _mm_mul_epu32(_mm_srli_si128(left.p, 4),
                                                _mm_srli_si128(right.p, 4));

    // Pack lower 32-bits of the 64-bit integers
    // into the result
    ret.p = _mm_unpacklo_epi32(_mm_shuffle_epi32(left13Right13, 0xe8),
                               _mm_shuffle_epi32(left24Right24, 0xe8));
  );

  T_NO_INTRIN (
    ret.v[0] = left.v[0] * right.v[0];
    ret.v[1] = left.v[1] * right.v[1];
    ret.v[2] = left.v[2] * right.v[2];
    ret.v[3] = left.v[3] * right.v[3];
  );

  return ret;
}

// operator/
static inline vec4 operator/(const vec4 &left, const vec4 &right) {
  vec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_div_ps(left.p, right.p);
  );

  T_NO_INTRIN (
    ret.v[0] = left.v[0] / right.v[0];
    ret.v[1] = left.v[1] / right.v[1];
    ret.v[2] = left.v[2] / right.v[2];
    ret.v[3] = left.v[3] / right.v[3];
  );

  return ret;
}
static inline ivec4 operator/(const ivec4 &left, const ivec4 &right) {
  ivec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_cvttps_epi32(_mm_div_ps(_mm_cvtepi32_ps(left.p),
                                        _mm_cvtepi32_ps(right.p)));
  );

  T_NO_INTRIN (
    ret.v[0] = left.v[0] / right.v[0];
    ret.v[1] = left.v[1] / right.v[1];
    ret.v[2] = left.v[2] / right.v[2];
    ret.v[3] = left.v[3] / right.v[3];
  );

  return ret;
}

// operator&
static inline vec4 operator&(const vec4 &left, const vec4 &right) {
  vec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_and_ps(left.p, right.p);
  );

  T_NO_INTRIN (
    ret.i[0] = left.i[0] & right.i[0];
    ret.i[1] = left.i[1] & right.i[1];
    ret.i[2] = left.i[2] & right.i[2];
    ret.i[3] = left.i[3] & right.i[3];
  );

  return ret;
}
static inline ivec4 operator&(const ivec4 &left, const ivec4 &right) {
  ivec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_and_si128(left.p, right.p);
  );

  T_NO_INTRIN (
    ret.v[0] = left.v[0] & right.v[0];
    ret.v[1] = left.v[1] & right.v[1];
    ret.v[2] = left.v[2] & right.v[2];
    ret.v[3] = left.v[3] & right.v[3];
  );

  return ret;
}

// operator|
static inline vec4 operator|(const vec4 &left, const vec4 &right) {
  vec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_or_ps(left.p, right.p);
  );

  T_NO_INTRIN (
    ret.i[0] = left.i[0] | right.i[0];
    ret.i[1] = left.i[1] | right.i[1];
    ret.i[2] = left.i[2] | right.i[2];
    ret.i[3] = left.i[3] | right.i[3];
  );

  return ret;
}
static inline ivec4 operator|(const ivec4 &left, const ivec4 &right) {
  ivec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_or_si128(left.p, right.p);
  );

  T_NO_INTRIN (
    ret.v[0] = left.v[0] | right.v[0];
    ret.v[1] = left.v[1] | right.v[1];
    ret.v[2] = left.v[2] | right.v[2];
    ret.v[3] = left.v[3] | right.v[3];
  );

  return ret;
}

// operator^
static inline vec4 operator^(const vec4 &left, const vec4 &right) {
  vec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_xor_ps(left.p, right.p);
  );

  T_NO_INTRIN (
    ret.i[0] = left.i[0] ^ right.i[0];
    ret.i[1] = left.i[1] ^ right.i[1];
    ret.i[2] = left.i[2] ^ right.i[2];
    ret.i[3] = left.i[3] ^ right.i[3];
  );

  return ret;
}
static inline ivec4 operator^(const ivec4 &left, const ivec4 &right) {
  ivec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_xor_si128(left.p, right.p);
  );

  T_NO_INTRIN (
    ret.v[0] = left.v[0] ^ right.v[0];
    ret.v[1] = left.v[1] ^ right.v[1];
    ret.v[2] = left.v[2] ^ right.v[2];
    ret.v[3] = left.v[3] ^ right.v[3];
  );

  return ret;
}

// operator<<
static inline vec4 operator<<(const vec4 &vec, int shift) {
  vec4 ret;

  T_SSE_INTRIN (
    ret.ip = _mm_sll_epi32(vec.ip, _mm_cvtsi32_si128(shift));
  );

  T_NO_INTRIN (
    ret.i[0] = vec.i[0] << shift;
    ret.i[1] = vec.i[1] << shift;
    ret.i[2] = vec.i[2] << shift;
    ret.i[3] = vec.i[3] << shift;
  );

  return ret;
}
static inline ivec4 operator<<(const ivec4 &vec, int shift) {
  ivec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_sll_epi32(vec.p, _mm_cvtsi32_si128(shift));
  );

  T_NO_INTRIN (
    ret.v[0] = vec.v[0] << shift;
    ret.v[1] = vec.v[1] << shift;
    ret.v[2] = vec.v[2] << shift;
    ret.v[3] = vec.v[3] << shift;
  );

  return ret;
}

// operator>>
static inline vec4 operator>>(const vec4 &vec, int shift) {
  vec4 ret;

  T_SSE_INTRIN (
    ret.ip = _mm_srl_epi32(vec.ip, _mm_cvtsi32_si128(shift));
  );

  T_NO_INTRIN (
    ret.i[0] = vec.i[0] >> shift;
    ret.i[1] = vec.i[1] >> shift;
    ret.i[2] = vec.i[2] >> shift;
    ret.i[3] = vec.i[3] >> shift;
  );

  return ret;
}
static inline ivec4 operator>>(const ivec4 &vec, int shift) {
  ivec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_srl_epi32(vec.p, _mm_cvtsi32_si128(shift));
  );

  T_NO_INTRIN (
    ret.v[0] = vec.v[0] >> shift;
    ret.v[1] = vec.v[1] >> shift;
    ret.v[2] = vec.v[2] >> shift;
    ret.v[3] = vec.v[3] >> shift;
  );

  return ret;
}

// operator+= (and all the others)
static inline vec4 &operator+=(vec4 &left, const vec4 &right) {return left = left + right;}
static inline ivec4 &operator+=(ivec4 &left, const ivec4 &right) {return left = left + right;}
static inline vec4 &operator-=(vec4 &left, const vec4 &right) {return left = left - right;}
static inline ivec4 &operator-=(ivec4 &left, const ivec4 &right) {return left = left - right;}
static inline vec4 &operator*=(vec4 &left, const vec4 &right) {return left = left * right;}
static inline ivec4 &operator*=(ivec4 &left, const ivec4 &right) {return left = left * right;}
static inline vec4 &operator/=(vec4 &left, const vec4 &right) {return left = left / right;}
static inline ivec4 &operator/=(ivec4 &left, const ivec4 &right) {return left = left / right;}
static inline vec4 &operator&=(vec4 &left, const vec4 &right) {return left = left & right;}
static inline ivec4 &operator&=(ivec4 &left, const ivec4 &right) {return left = left & right;}
static inline vec4 &operator|=(vec4 &left, const vec4 &right) {return left = left | right;}
static inline ivec4 &operator|=(ivec4 &left, const ivec4 &right) {return left = left | right;}
static inline vec4 &operator^=(vec4 &left, const vec4 &right) {return left = left ^ right;}
static inline ivec4 &operator^=(ivec4 &left, const ivec4 &right) {return left = left ^ right;}
static inline vec4 &operator<<=(vec4 &vec, int shift) {return vec = vec << shift;}
static inline ivec4 &operator<<=(ivec4 &vec, int shift) {return vec = vec << shift;}
static inline vec4 &operator>>=(vec4 &vec, int shift) {return vec = vec >> shift;}
static inline ivec4 &operator>>=(ivec4 &vec, int shift) {return vec = vec >> shift;}

// ToVec4/ToIvec4
static inline vec4 ToVec4(const ivec4 &other) {
  vec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_cvtepi32_ps(other.p);
  );

  T_NO_INTRIN (
    ret.v[0] = other.v[0];
    ret.v[1] = other.v[1];
    ret.v[2] = other.v[2];
    ret.v[3] = other.v[3];
  );

  return ret;
}
static inline ivec4 ToIvec4(const vec4 &other) {
  ivec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_cvttps_epi32(other.p);
  );

  T_NO_INTRIN (
    ret.v[0] = other.v[0];
    ret.v[1] = other.v[1];
    ret.v[2] = other.v[2];
    ret.v[3] = other.v[3];
  );

  return ret;
}

// Vec4Cast/Ivec4Cast
static inline vec4 Vec4Cast(const ivec4 &other) {
  vec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_load_ps(other.f);
  );

  T_NO_INTRIN (
    ret.h[0] = other.h[0];
    ret.h[1] = other.h[1];
  );

  return ret;
}
static inline ivec4 Ivec4Cast(const vec4 &other) {
  ivec4 ret;

  T_SSE_INTRIN (
    ret.p = _mm_load_si128(&other.ip);
  );

  T_NO_INTRIN (
    ret.h[0] = other.h[0];
    ret.h[1] = other.h[1];
  );

  return ret;
}

// operator~
static inline vec4 operator~(const vec4 &vec) {
  return vec^VEC4_U_1(0xffffffff);
}
static inline ivec4 operator~(const ivec4 &vec) {
  return vec^IVEC4_U_1(0xffffffff);
}

// unary operator-
static inline vec4 operator-(const vec4 &vec) {
  return vec^VEC4_U_1(0x80000000);
}
static inline ivec4 operator-(const ivec4 &vec) {
  return ~vec + IVEC4_1(1);
}

// operator==
static inline bool operator==(const vec4 &left, const vec4 &right) {
  T_SSE_INTRIN (
    return _mm_movemask_ps(_mm_cmpeq_ps(left.p, right.p)) == 0xf;
  );

  T_NO_INTRIN (
    return ((left.h[0] == right.h[0]) &&
            (left.h[1] == right.h[1]));
  );
}
static inline bool operator==(const ivec4 &left, const ivec4 &right) {
  T_SSE_INTRIN (
    return _mm_movemask_epi8(_mm_cmpeq_epi32(left.p, right.p)) == 0xffff;
  );

  T_NO_INTRIN (
    return ((left.h[0] == right.h[0]) &&
            (left.h[1] == right.h[1]));
  );
}

// operator!=
static inline bool operator!=(const vec4 &left, const vec4 &right) {
  T_SSE_INTRIN (
    return _mm_movemask_ps(_mm_cmpeq_ps(left.p, right.p)) != 0xf;
  );

  T_NO_INTRIN (
    return ((left.h[0] != right.h[0]) ||
            (left.h[1] != right.h[1]));
  );
}
static inline bool operator!=(const ivec4 &left, const ivec4 &right) {
  T_SSE_INTRIN (
    return _mm_movemask_epi8(_mm_cmpeq_epi32(left.p, right.p)) != 0xffff;
  );

  T_NO_INTRIN (
    return ((left.h[0] != right.h[0]) ||
            (left.h[1] != right.h[1]));
  );
}

#undef T_SSE_INTRIN
#undef T_NO_INTRIN

#endif //_LOVEYLIB_VECTOR_H
