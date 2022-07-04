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
 * src/loveylib/canvas.h:
 *  Platform-dependent rendering canvas
 *
 ************************************************************/

#ifndef _LOVEYLIB_CANVAS_H
#define _LOVEYLIB_CANVAS_H

#include "loveylib/types.h"
#include "loveylib/event.h"
#include "loveylib/opengl.h"
#include "loveylib_config.h"

enum canvas_api_e : ufast {
  CANVAS_SOFTWARE_API, // Software canvas
  CANVAS_OPENGL_API, // OpenGL canvas

  CANVAS_API_COUNT
};
typedef ufast canvas_api_t;

// Canvas forward declaration
union canvas_t;

// Canvas data size
static constexpr const uptr CANVAS_DATA_SIZE = 528;

// Canvas vtable

// Get event from canvas
// Returns false if no event is available
typedef bfast (*canvas_poll_func_t)(canvas_t */*data*/, event_t */*out*/);

// Render canvas
// Returns false if canvas has been closed
typedef void (*canvas_render_func_t)(canvas_t */*data*/);

// Close canvas, invalidates data
typedef void (*canvas_close_func_t)(canvas_t */*data*/);

struct canvas_funcs_t {
  canvas_poll_func_t pollEvent;
  canvas_render_func_t render;
  canvas_close_func_t close;
};

// Canvas base data
struct canvas_base_t {
  const canvas_funcs_t *f;
  canvas_api_t api;
};
static_assert(sizeof(canvas_base_t) <= CANVAS_DATA_SIZE, "");

// Software canvas public data
struct software_canvas_data_t {
  canvas_base_t b;

  // Surface size
  uptr width, height;

  // Surface stride
  uptr stride;

  // Output surface, BGRA format
  u32 *surface; // size = stride*height
};
static_assert(sizeof(software_canvas_data_t) <= CANVAS_DATA_SIZE, "");

// OpenGL canvas public data
struct gl_canvas_data_t {
  canvas_base_t b;

  gl::funcs_t f; // OpenGL function table
};

// Public canvas data
union canvas_data_t {
  canvas_base_t base;
  software_canvas_data_t software;
  gl_canvas_data_t gl;
};
static_assert(sizeof(canvas_data_t) <= CANVAS_DATA_SIZE, "");

// Canvas
union canvas_t {
  canvas_data_t c;

  u8 data[CANVAS_DATA_SIZE];
};
static_assert(sizeof(canvas_t) == CANVAS_DATA_SIZE, "");

// Convenience vtable routines
static inline bfast PollCanvasEvent(canvas_t *c, event_t *out) {
  return c->c.base.f->pollEvent(c, out);
}
static inline void RenderCanvas(canvas_t *c) {
  c->c.base.f->render(c);
}
static inline void CloseCanvas(canvas_t *c) {
  return c->c.base.f->close(c);
}

// Create software canvas
// Returns false on failure
bfast CreateSoftwareCanvas(canvas_t *out, const char *title, u32 width, u32 height);

// Create OpenGL canvas
// Returns false on failure
// When created, it will automatically become the active canvas
bfast CreateOpenGLCanvas(canvas_t *out, const char *title, u32 width, u32 height);

#endif //_LOVEYLIB_CANVAS_H
