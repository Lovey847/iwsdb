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

#ifndef _DRAW_H
#define _DRAW_H

#include "loveylib/types.h"
#include "loveylib/vector.h"
#include "loveylib/canvas.h"
#include "game.h"
#include "vertex.h"

// Certain pages have variations on certain images
typedef ifast page_t;
static constexpr const page_t NUM_PAGES = 2;

// Create window for drawing
void CreateWindow(canvas_t *out, const char *title);

// Destroy window
void CloseWindow(canvas_t *c);

// Set image page
void SetPage(page_t p);

// Draw image at position, with scale
//
// Argument requirements:
//  pos.w == <0>
//  scale.zw == <1 1>
void DrawImage(vec4 pos, vec4 scale, image_id_t img);

// Draw quads to screen
void DrawQuads(const rquad_t *quads, uptr quadCount);

// Set renderer clear color
void SetClearColor(f32 r, f32 g, f32 b);

// TODO: Write a function to render a single entity, when those are a thing!
// I wanna see if that's faster than rendering all of them in bulk
void RenderGame();

#endif //_DRAW_H
