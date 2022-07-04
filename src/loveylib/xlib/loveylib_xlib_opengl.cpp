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
 * src/loveylib/xlib/loveylib_xlib_opengl.cpp:
 *  X11 OpenGL interface loader
 *
 ************************************************************/

#include "loveylib_config.h"

// Make sure we have OpenGL
#ifdef LOVEYLIB_OPENGL

#include "loveylib/types.h"
#include "loveylib/opengl.h"

#include <GL/glx.h>

bfast gl::LoadFuncs(gl::funcs_t *out) {
  // Load opengl functions
#define T_OPENGL_FUNC_DEF(_type, _name, ...)                            \
  out->_name = (gl::_name ## _t)glXGetProcAddress((GLubyte*)"gl" #_name); \
  if (!out->_name) return false;
  {OPENGL_FUNC_LIST}
#undef T_OPENGL_FUNC_DEF

  return true;
}

void gl::FreeFuncs(gl::funcs_t *f) {(void)f;}

#endif //LOVEYLIB_OPENGL
