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
 * This file is part of I Wanna Slay the Dragon of Bangan
 *
 ************************************************************/

#ifndef _VERTEX_H
#define _VERTEX_H

#include "loveylib/types.h"
#include "loveylib/vector.h"

// Render vertex
struct vertex_t {
  union {
    // Vertex data
    struct {
      f32 x, y, z;

      u16 s, t;
    } data;

    // xyz = vertex position
    vec4 pos;

    // w = texture coordinate
    struct {
      u32 pad[3];
      u16 x, y;
    } coord;
  };
};
static_assert(sizeof(vertex_t) == 16, "");

// Rendering quad
// Aligned to cacheline boundary
struct alignas(64) rquad_t {
  // top left, top right, bottom left, bottom right
  vertex_t v[4];
};

#endif //_VERTEX_H
