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
 * src/loveylib/opengl.h:
 *  OpenGL interface
 *
 ************************************************************/

#ifndef _LOVEYLIB_OPENGL_H
#define _LOVEYLIB_OPENGL_H

#include "loveylib/types.h"
#include "loveylib_config.h"

// OpenGL function calling convention
// Platform & compiler specific
#ifdef LOVEYLIB_WIN32

#ifdef LOVEYLIB_MSVC
#define OPENGL_CONVENTION __stdcall
#elif defined(LOVEYLIB_GNU)
#define OPENGL_CONVENTION __attribute__((stdcall))
#endif

#else //LOVEYLIB_WIN32

#define OPENGL_CONVENTION

#endif //LOVEYLIB_WIN32

// OpenGL function list
// Define T_OPENGL_FUNC_DEF before using
#define OPENGL_FUNC_LIST                                                \
  T_OPENGL_FUNC_DEF(void, Clear, ::gl::bitfield_t)                      \
                                                                        \
  T_OPENGL_FUNC_DEF(void, ClearColor, ::gl::clampf_t, ::gl::clampf_t,   \
                    ::gl::clampf_t, ::gl::clampf_t)                     \
                                                                        \
  T_OPENGL_FUNC_DEF(void, Viewport, ::gl::int_t, ::gl::int_t,           \
                    ::gl::sizei_t, ::gl::sizei_t)                       \
                                                                        \
  T_OPENGL_FUNC_DEF(void, GenVertexArrays, ::gl::sizei_t, ::gl::uint_t*) \
                                                                        \
  T_OPENGL_FUNC_DEF(void, DeleteVertexArrays, ::gl::sizei_t, ::gl::uint_t*) \
                                                                        \
  T_OPENGL_FUNC_DEF(::gl::enum_t, GetError, void)                       \
                                                                        \
  T_OPENGL_FUNC_DEF(::gl::uint_t, CreateProgram, void)                  \
                                                                        \
  T_OPENGL_FUNC_DEF(::gl::uint_t, CreateShader, ::gl::enum_t)           \
                                                                        \
  T_OPENGL_FUNC_DEF(void, ShaderSource, ::gl::uint_t, ::gl::sizei_t,    \
                    const ::gl::char_t**, const ::gl::int_t*)           \
                                                                        \
  T_OPENGL_FUNC_DEF(void, CompileShader, ::gl::uint_t)                  \
                                                                        \
  T_OPENGL_FUNC_DEF(void, GetShaderiv, ::gl::uint_t, ::gl::enum_t,      \
                    ::gl::int_t*)                                       \
                                                                        \
  T_OPENGL_FUNC_DEF(void, GetShaderInfoLog, ::gl::uint_t, ::gl::sizei_t, \
                    ::gl::sizei_t*, ::gl::char_t*)                      \
                                                                        \
  T_OPENGL_FUNC_DEF(void, DeleteShader, ::gl::uint_t)                   \
                                                                        \
  T_OPENGL_FUNC_DEF(void, DrawElements, ::gl::enum_t, ::gl::sizei_t,    \
                    ::gl::enum_t, const void*)                          \
                                                                        \
  T_OPENGL_FUNC_DEF(void, DrawArrays, ::gl::enum_t, ::gl::int_t, ::gl::sizei_t) \
                                                                        \
  T_OPENGL_FUNC_DEF(void, BufferSubData, ::gl::enum_t, ::gl::intptr_t,  \
                    ::gl::sizeiptr_t, const void*)                      \
                                                                        \
  T_OPENGL_FUNC_DEF(void, BufferData, ::gl::enum_t, ::gl::sizeiptr_t,   \
                    const void*, ::gl::enum_t)                          \
                                                                        \
  T_OPENGL_FUNC_DEF(void, UseProgram, ::gl::uint_t)                     \
                                                                        \
  T_OPENGL_FUNC_DEF(void, EnableVertexAttribArray, ::gl::uint_t)        \
                                                                        \
  T_OPENGL_FUNC_DEF(void, VertexAttribPointer, ::gl::uint_t, ::gl::int_t, \
                    ::gl::enum_t, ::gl::boolean_t, ::gl::sizei_t,       \
                    const void*)                                        \
                                                                        \
  T_OPENGL_FUNC_DEF(void, BindBuffer, ::gl::enum_t, ::gl::uint_t)       \
                                                                        \
  T_OPENGL_FUNC_DEF(void, GenBuffers, ::gl::sizei_t, ::gl::uint_t*)     \
                                                                        \
  T_OPENGL_FUNC_DEF(void, BindVertexArray, ::gl::uint_t)                \
                                                                        \
  T_OPENGL_FUNC_DEF(void, DeleteProgram, ::gl::uint_t)                  \
                                                                        \
  T_OPENGL_FUNC_DEF(void, GetProgramInfoLog, ::gl::uint_t, ::gl::sizei_t, \
                    ::gl::sizei_t*, ::gl::char_t*)                      \
                                                                        \
  T_OPENGL_FUNC_DEF(void, GetProgramiv, ::gl::uint_t, ::gl::enum_t,     \
                    ::gl::int_t*)                                       \
                                                                        \
  T_OPENGL_FUNC_DEF(void, DetachShader, ::gl::uint_t, ::gl::uint_t)     \
                                                                        \
  T_OPENGL_FUNC_DEF(void, LinkProgram, ::gl::uint_t)                    \
                                                                        \
  T_OPENGL_FUNC_DEF(void, AttachShader, ::gl::uint_t, ::gl::uint_t)     \
                                                                        \
  T_OPENGL_FUNC_DEF(void, DeleteBuffers, ::gl::sizei_t, const ::gl::uint_t*) \
                                                                        \
  T_OPENGL_FUNC_DEF(void*, MapBufferRange, ::gl::enum_t, ::gl::intptr_t, \
                    ::gl::sizeiptr_t, ::gl::bitfield_t)                 \
                                                                        \
  T_OPENGL_FUNC_DEF(::gl::boolean_t, UnmapBuffer, ::gl::enum_t)         \
                                                                        \
  T_OPENGL_FUNC_DEF(void, GenTextures, ::gl::sizei_t, ::gl::uint_t*)    \
                                                                        \
  T_OPENGL_FUNC_DEF(void, BindTexture, ::gl::enum_t, ::gl::uint_t)      \
                                                                        \
  T_OPENGL_FUNC_DEF(void, ActiveTexture, ::gl::enum_t)                  \
                                                                        \
  T_OPENGL_FUNC_DEF(void, TexImage2D, ::gl::enum_t, ::gl::int_t,        \
                    ::gl::int_t, ::gl::sizei_t, ::gl::sizei_t, ::gl::int_t, \
                    ::gl::enum_t, ::gl::enum_t, const void*)            \
                                                                        \
  T_OPENGL_FUNC_DEF(void, TexSubImage2D, ::gl::enum_t, ::gl::int_t,     \
                    ::gl::int_t, ::gl::int_t, ::gl::sizei_t, ::gl::sizei_t, \
                    ::gl::enum_t, ::gl::enum_t, const void*)            \
                                                                        \
  T_OPENGL_FUNC_DEF(void, DeleteTextures, ::gl::sizei_t, const ::gl::uint_t*) \
                                                                        \
  T_OPENGL_FUNC_DEF(void, TexParameteri, ::gl::enum_t, ::gl::enum_t, ::gl::int_t) \
                                                                        \
  T_OPENGL_FUNC_DEF(void, Enable, ::gl::enum_t)                         \
                                                                        \
  T_OPENGL_FUNC_DEF(void, BlendFunc, ::gl::enum_t, ::gl::enum_t)        \
                                                                        \
  T_OPENGL_FUNC_DEF(void, DepthFunc, ::gl::enum_t)                      \
                                                                        \
  T_OPENGL_FUNC_DEF(void, DepthRange, ::gl::double_t, ::gl::double_t)   \
                                                                        \
  T_OPENGL_FUNC_DEF(void, Scissor, ::gl::int_t, ::gl::int_t, ::gl::sizei_t, \
                    ::gl::sizei_t)                                      \
                                                                        \
  T_OPENGL_FUNC_DEF(void, GetFloatv, ::gl::enum_t, ::gl::float_t*)      \
                                                                        \
  T_OPENGL_FUNC_DEF(void, DrawBuffer, ::gl::enum_t)

namespace gl {

// Types
typedef i8 boolean_t;
typedef i8 byte_t;
typedef i16 short_t;
typedef i16 half_t;
typedef i32 int_t;
typedef i32 fixed_t;
typedef i32 sizei_t;
typedef i64 int64_t;
typedef iptr intptr_t;
typedef iptr sizeiptr_t;
typedef u8 ubyte_t;
typedef u16 ushort_t;
typedef u32 uint_t;
typedef u32 enum_t;
typedef u32 bitfield_t;
typedef u64 uint64_t;
typedef uptr sync_t;
typedef f32 float_t;
typedef f32 clampf_t;
typedef f64 double_t;
typedef f64 clampd_t;
typedef char char_t;
typedef void void_t;

// Enum values
enum enum_e : enum_t {
  COLOR_BUFFER_BIT = 0x4000,
  DEPTH_BUFFER_BIT = 0x0100,
  STENCIL_BUFFER_BIT = 0x0400,
  VERTEX_SHADER = 0x8b31,
  FRAGMENT_SHADER = 0x8b30,
  COMPILE_STATUS = 0x8b81,
  LINK_STATUS = 0x8b82,
  NO_ERROR = 0,
  UNSIGNED_SHORT = 0x1403,
  FLOAT = 0x1406,
  TRIANGLES = 0x0004,
  ARRAY_BUFFER = 0x8892,
  STREAM_DRAW = 0x88e0,
  ELEMENT_ARRAY_BUFFER = 0x8893,
  STATIC_DRAW = 0x88e4,
  UNSIGNED_BYTE = 0x1401,
  UNSIGNED_INT = 0x1405,
  MAP_READ_BIT = 0x0001,
  MAP_WRITE_BIT = 0x0002,
  MAP_PERSISTENT_BIT = 0x0040,
  MAP_COHERENT_BIT = 0x0080,
  MAP_INVALIDATE_RANGE_BIT = 0x0004,
  MAP_INVALIDATE_BUFFER_BIT = 0x0008,
  MAP_FLUSH_EXPLICIT_BIT = 0x0010,
  MAP_UNSYNCHRONIZED_BIT = 0x0020,
  TEXTURE_MIN_FILTER = 0x2801,
  TEXTURE_MAG_FILTER = 0x2800,
  NEAREST = 0x2600,
  LINEAR = 0x2601,
  TEXTURE_MAX_LEVEL = 0x813d,
  TEXTURE_WRAP_S = 0x2802,
  TEXTURE_WRAP_T = 0x2803,
  CLAMP_TO_EDGE = 0x812f,
  TEXTURE_2D = 0x0de1,
  TEXTURE0 = 0x84c0,
  BGRA = 0x80e1,
  RGBA8 = 0x8058,
  BLEND = 0x0be2,
  SRC_ALPHA = 0x0302,
  ONE_MINUS_SRC_ALPHA = 0x0303,
  DEPTH_TEST = 0x0b71,
  GREATER = 0x0204,
  GEQUAL = 0x0206,
  LESS = 0x0201,
  SCISSOR_TEST = 0x0c11,
  COLOR_CLEAR_VALUE = 0x0c22,
  FRONT_AND_BACK = 0x0408,
  BACK = 0x0405,
};

// OpenGL function types
#define T_OPENGL_FUNC_DEF(_type, _name, ...)                    \
  typedef _type (OPENGL_CONVENTION *_name ## _t)(__VA_ARGS__);
OPENGL_FUNC_LIST
#undef T_OPENGL_FUNC_DEF

// OpenGL function table
#define T_OPENGL_FUNC_DEF(_type, _name, ...) _name ## _t _name;
struct funcs_t {
  void *res; // Pointer to additional resources, if needed

  OPENGL_FUNC_LIST
};
#undef T_OPENGL_FUNC_DEF

// Load function table
// Returns false on error
bfast LoadFuncs(funcs_t *out);

// Free function table resources
void FreeFuncs(funcs_t *f);

}

#endif //_LOVEYLIB_OPENGL_H
