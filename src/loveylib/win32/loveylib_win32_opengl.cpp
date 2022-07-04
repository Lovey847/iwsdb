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
 * src/loveylib/win32/loveylib_win32_opengl.cpp:
 *  Win32 OpenGL interface loader
 *
 ************************************************************/

#include "loveylib_config.h"

// Make sure we have OpenGL
#ifdef LOVEYLIB_OPENGL

#include "loveylib/types.h"
#include "loveylib/opengl.h"
#include "loveylib/win32/loveylib_wgl.h"

#include <cstring>

// Check validity of function returned by wglGetProcAddress
bfast ValidFunc(void *f) {
  uptr v = (uptr)f;
  return (v > 3) && (v != -(uptr)1);
}

// Get opengl32.dll
static bptr InitLoadFunc(gl::funcs_t *f) {
  f->res = (void*)win32::GetModuleHandle("opengl32.dll");
  return (bptr)f->res;
}

// Get any OpenGL function
static void *LoadFunc(gl::funcs_t *f, const char *funcName) {
  void *ret = win32::wglGetProcAddress(funcName);
  if (!ValidFunc(ret))
    ret = win32::GetProcAddress((win32::hmodule_t)f->res, funcName);

  return ret;
}

bfast gl::LoadFuncs(gl::funcs_t *out) {
  InitLoadFunc(out);

  // Load opengl functions
#define T_OPENGL_FUNC_DEF(_type, _name, ...)                \
  out->_name = (gl::_name ## _t)LoadFunc(out, "gl" #_name); \
  if (!out->_name) return false;
  {OPENGL_FUNC_LIST}
#undef T_OPENGL_FUNC_DEF

  return true;
}

void gl::FreeFuncs(gl::funcs_t *f) {(void)f;}

// WGL extensions
typedef const char *(OPENGL_CONVENTION *PFNWGLGETEXTENSIONSSTRINGARBPROC)(win32::hdc_t hdc);
typedef win32::hglrc_t (OPENGL_CONVENTION *PFNWGLCREATECONTEXTATTRIBSARBPROC)(
  win32::hdc_t, win32::hglrc_t, const int*
  );
typedef b32 (OPENGL_CONVENTION *PFNWGLSWAPINTERVALEXTPROC)(int);

bfast wgl::LoadExt(wgl::funcs_t *out, win32::hdc_t hdc) {
  // Get WGL extension list
  PFNWGLGETEXTENSIONSSTRINGARBPROC GetExtensionsStringARB =
    (PFNWGLGETEXTENSIONSSTRINGARBPROC)win32::wglGetProcAddress("wglGetExtensionsStringARB");
  if (!ValidFunc(GetExtensionsStringARB)) return false;

  const char *extStr = GetExtensionsStringARB(hdc);

  // Check for required extensions in extension list
  if (!strstr(extStr, "WGL_ARB_create_context_profile")) return false;

  // Load required extensions
  out->CreateContextAttribsARB = win32::wglGetProcAddress("wglCreateContextAttribsARB");
  if (!ValidFunc(out->CreateContextAttribsARB)) return false;

  // Load optional extensions
  if (!strstr(extStr, "WGL_EXT_swap_control")) out->SwapIntervalEXT = NULL;
  else out->SwapIntervalEXT = win32::wglGetProcAddress("wglSwapIntervalEXT");

  // All done here
  return true;
}

// Extension function wrappers
win32::hglrc_t wgl::CreateContextAttribsARB(const wgl::funcs_t *f, win32::hdc_t hdc,
                                            win32::hglrc_t shareCtx, const int *attribs)
{
  PFNWGLCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB =
    (PFNWGLCREATECONTEXTATTRIBSARBPROC)f->CreateContextAttribsARB;
  return CreateContextAttribsARB(hdc, shareCtx, attribs);
}

b32 wgl::SwapIntervalEXT(const wgl::funcs_t *f, int interval) {
  PFNWGLSWAPINTERVALEXTPROC SwapIntervalEXT =
    (PFNWGLSWAPINTERVALEXTPROC)f->SwapIntervalEXT;
  return SwapIntervalEXT(interval);
}

#endif //LOVEYLIB_OPENGL
