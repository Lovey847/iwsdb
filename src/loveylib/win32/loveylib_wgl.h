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
 * src/loveylib/win32/loveylib_wgl.h
 *  WGL extension interface
 *  Base WGL functions are stored in loveylib_windows.h
 *
 ************************************************************/

#ifndef _LOVEYLIB_WIN32_LOVEYLIB_WGL_H
#define _LOVEYLIB_WIN32_LOVEYLIB_WGL_H

#include "loveylib/win32/loveylib_windows.h"

namespace wgl {
  // WGL extension function list
  // (base WGL funcitons
  struct funcs_t {
    void *CreateContextAttribsARB;
    void *SwapIntervalEXT;
  };

  // Load WGL extension functions
  // Returns false if required extensions weren't found
  bfast LoadExt(funcs_t *out, win32::hdc_t hdc);

  // Required functions
  win32::hglrc_t CreateContextAttribsARB(const funcs_t *f, win32::hdc_t hdc,
                                         win32::hglrc_t shareCtx, const int *attribs);

  // Optional functions
  b32 SwapIntervalEXT(const funcs_t *f, int interval);
};

#endif //_LOVEYLIB_WIN32_LOVEYLIB_WGL_H
