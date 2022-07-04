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
 * src/loveylib/xlib/loveylib_win32_canvas.cpp:
 *  Win32 canvas interface
 *
 ************************************************************/

#include "loveylib/types.h"
#include "loveylib/canvas.h"
#include "loveylib/assert.h"
#include "loveylib/utils.h"
#include "loveylib/endian.h"
#include "loveylib/win32/loveylib_windows.h"
#include "loveylib/win32/loveylib_wgl.h"
#include "loveylib_config.h"
#include "log.h"
#include "game.h"

// Class name
static const char S_ClassName[] = "LoveyLib_Win32_Class_Name";

// Software canvas data
struct win32_canvas_data_t {
  // Public data
  canvas_data_t pub;

  win32::hwnd_t win;
  win32::hdc_t dev;

  event_t *evt; // Event to fill from WinProc
  bfast evtFilled; // Set to true when evt is filled

  // Press key code, for autorepeat
  key_code_t pressCode;

  bfast open; // Open status of window
};

// Get key code from virtual key
static key_code_t VKToKey(u32 vk, win32::lparam_t lp = 0) {
  switch (vk) {
  case win32::VK_CONTROL:
    // If the key is extended, that means it's right control
    // Otherwise, it's left control
    return KEYC_LCTRL + ((lp&0x1000000)>>24);

  case win32::VK_MENU:
    return KEYC_LALT + ((lp&0x1000000)>>24);

  case win32::VK_SHIFT:
    // For shift, we have to check the scan code
    // VK_SHIFT should only send either 0x2a (left shift)
    // or 0x36 (right shift), so I can do this without
    // a comparison
    return KEYC_LSHIFT + ((lp&0x100000)>>20);

  case win32::VK_SNAPSHOT: return KEYC_PRINTSCR;
  case win32::VK_PAUSE: return KEYC_PAUSEBRK;

  case win32::VK_BACK: return KEYC_BACKSPACE;
  case win32::VK_TAB: return KEYC_TAB;
  case win32::VK_RETURN:
    // If it's an extended key, VK_RETURN is on
    // the number pad
    if (lp&0x1000000) return KEYC_NUM_ENTER;
    else return KEYC_RETURN;

  case win32::VK_INSERT: return KEYC_INSERT;
  case win32::VK_HOME: return KEYC_HOME;
  case win32::VK_END: return KEYC_END;
  case win32::VK_PRIOR: return KEYC_PAGEUP;
  case win32::VK_NEXT: return KEYC_PAGEDOWN;

  case win32::VK_UP: return KEYC_UP;
  case win32::VK_DOWN: return KEYC_DOWN;
  case win32::VK_LEFT: return KEYC_LEFT;
  case win32::VK_RIGHT: return KEYC_RIGHT;

  case win32::VK_ESCAPE: return KEYC_ESCAPE;

  case win32::VK_SPACE: return KEYC_SPACE;

  case win32::VK_OEM_7: return KEYC_APOSTROPHE;
  case win32::VK_OEM_COMMA: return KEYC_COMMA;
  case win32::VK_OEM_MINUS: return KEYC_HYPHEN;
  case win32::VK_OEM_PERIOD: return KEYC_PERIOD;
  case win32::VK_OEM_2: return KEYC_SLASH;

  case '0': return KEYC_0;
  case '1': return KEYC_1;
  case '2': return KEYC_2;
  case '3': return KEYC_3;
  case '4': return KEYC_4;
  case '5': return KEYC_5;
  case '6': return KEYC_6;
  case '7': return KEYC_7;
  case '8': return KEYC_8;
  case '9': return KEYC_9;

  case win32::VK_OEM_1: return KEYC_SEMICOLON;
  case win32::VK_OEM_PLUS: return KEYC_EQUALS;

  case win32::VK_F1:  return KEYC_F1;
  case win32::VK_F2:  return KEYC_F2;
  case win32::VK_F3:  return KEYC_F3;
  case win32::VK_F4:  return KEYC_F4;
  case win32::VK_F5:  return KEYC_F5;
  case win32::VK_F6:  return KEYC_F6;
  case win32::VK_F7:  return KEYC_F7;
  case win32::VK_F8:  return KEYC_F8;
  case win32::VK_F9:  return KEYC_F9;
  case win32::VK_F10:  return KEYC_F10;
  case win32::VK_F11:  return KEYC_F11;
  case win32::VK_F12:  return KEYC_F12;
  case win32::VK_F13:  return KEYC_F13;
  case win32::VK_F14:  return KEYC_F14;
  case win32::VK_F15:  return KEYC_F15;
  case win32::VK_F16:  return KEYC_F16;
  case win32::VK_F17:  return KEYC_F17;
  case win32::VK_F18:  return KEYC_F18;
  case win32::VK_F19:  return KEYC_F19;
  case win32::VK_F20:  return KEYC_F20;
  case win32::VK_F21:  return KEYC_F21;
  case win32::VK_F22:  return KEYC_F22;
  case win32::VK_F23:  return KEYC_F23;
  case win32::VK_F24:  return KEYC_F24;

  case win32::VK_OEM_4: return KEYC_OPENBRACKET;
  case win32::VK_OEM_5: return KEYC_BACKSLASH;
  case win32::VK_OEM_6: return KEYC_CLOSEBRACKET;
  case win32::VK_OEM_3: return KEYC_GRAVE;

  case 'A': return KEYC_A;
  case 'B': return KEYC_B;
  case 'C': return KEYC_C;
  case 'D': return KEYC_D;
  case 'E': return KEYC_E;
  case 'F': return KEYC_F;
  case 'G': return KEYC_G;
  case 'H': return KEYC_H;
  case 'I': return KEYC_I;
  case 'J': return KEYC_J;
  case 'K': return KEYC_K;
  case 'L': return KEYC_L;
  case 'M': return KEYC_M;
  case 'N': return KEYC_N;
  case 'O': return KEYC_O;
  case 'P': return KEYC_P;
  case 'Q': return KEYC_Q;
  case 'R': return KEYC_R;
  case 'S': return KEYC_S;
  case 'T': return KEYC_T;
  case 'U': return KEYC_U;
  case 'V': return KEYC_V;
  case 'W': return KEYC_W;
  case 'X': return KEYC_X;
  case 'Y': return KEYC_Y;
  case 'Z': return KEYC_Z;

  case win32::VK_DELETE: return KEYC_DELETE;

  case win32::VK_NUMPAD0: return KEYC_NUM0;
  case win32::VK_NUMPAD1: return KEYC_NUM1;
  case win32::VK_NUMPAD2: return KEYC_NUM2;
  case win32::VK_NUMPAD3: return KEYC_NUM3;
  case win32::VK_NUMPAD4: return KEYC_NUM4;
  case win32::VK_NUMPAD5: return KEYC_NUM5;
  case win32::VK_NUMPAD6: return KEYC_NUM6;
  case win32::VK_NUMPAD7: return KEYC_NUM7;
  case win32::VK_NUMPAD8: return KEYC_NUM8;
  case win32::VK_NUMPAD9: return KEYC_NUM9;

  case win32::VK_DIVIDE: return KEYC_NUM_DIVIDE;
  case win32::VK_MULTIPLY: return KEYC_NUM_MULTIPLY;
  case win32::VK_SUBTRACT: return KEYC_NUM_SUBTRACT;
  case win32::VK_ADD: return KEYC_NUM_ADD;

  case win32::VK_DECIMAL: return KEYC_NUM_DECIMAL;

  default: return KEYC_NONE;
  }
}

#define T_LOWORD(x) ((x)&0xffff)
#define T_HIWORD(x) (((x)&0xffff0000)>>16)
static win32::lresult_t WinProc(win32::hwnd_t win, u32 msg, win32::wparam_t wp, win32::lparam_t lp) {
  win32_canvas_data_t *c;
  u32 test;

  switch (msg) {
    // FANGAME HACK, DON'T MERGE INTO LOVEYLIB
  case win32::WM_SIZE:
    c = (win32_canvas_data_t*)win32::GetWindowLongPtr(win, win32::GWLP_USERDATA);

    // Set viewport if we're in OpenGL
    if ((c->pub.base.api == CANVAS_OPENGL_API) && c->pub.gl.f.res) {
      const int winWidth = T_LOWORD(lp);
      const int winHeight = T_HIWORD(lp);
      int x, y, width, height;
      width = winWidth;
      height = winHeight;

      f32 color[4];
      c->pub.gl.f.GetFloatv(gl::COLOR_CLEAR_VALUE, color);

      c->pub.gl.f.Scissor(0, 0, width, height);
      c->pub.gl.f.DrawBuffer(gl::FRONT_AND_BACK);
      c->pub.gl.f.ClearColor(0, 0, 0, 0);
      c->pub.gl.f.Clear(gl::COLOR_BUFFER_BIT);
      c->pub.gl.f.DrawBuffer(gl::BACK);
      c->pub.gl.f.ClearColor(color[0], color[1], color[2], color[3]);

      const f32 ratio = (f32)width/(f32)height;
      constexpr f32 GAME_RATIO = (f32)GAME_WIDTH/(f32)GAME_HEIGHT;
      if (ratio >= GAME_RATIO) {
        width = (f32)height*GAME_RATIO;
        x = (winWidth-width)/2;
        y = 0;
      } else {
        height = (f32)width/GAME_RATIO;
        x = 0;
        y = (winHeight-height)/2;
      }

      c->pub.gl.f.Viewport(x, y, width, height);
      c->pub.gl.f.Scissor(x, y, width, height);
    }
    break;

  case win32::WM_NCCREATE: {
    win32::create_struct_t * const createParams = (win32::create_struct_t*)lp;
    iptr p = (iptr)createParams->createParams;

    win32::SetWindowLongPtr(win, win32::GWLP_USERDATA, p);

    break;
  }
  case win32::WM_CLOSE:
    c = (win32_canvas_data_t*)win32::GetWindowLongPtr(win, win32::GWLP_USERDATA);

    c->evt->type = CLOSE_EVENT;
    c->evtFilled = true;
    return 0;

  case win32::WM_DESTROY:
    c = (win32_canvas_data_t*)win32::GetWindowLongPtr(win, win32::GWLP_USERDATA);
    c->open = false;
    break;

  case win32::WM_KEYDOWN:
  case win32::WM_SYSKEYDOWN:
    // Retrieve canvas pointer
    c = (win32_canvas_data_t*)win32::GetWindowLongPtr(win, win32::GWLP_USERDATA);

    // FANGAME HACK, DON'T IMPORT INTO LOVEYLIB
    // If this is alt+enter, toggle fullscreen
    if ((lp&0x20000000) && (wp == win32::VK_RETURN)) {
      static bfast s_fullscreen = false;

      if (!s_fullscreen) {
        int width, height;

        win32::hmonitor_t monitor = win32::MonitorFromWindow(win, win32::MONITOR_DEFAULTTONEAREST);
        win32::monitor_info_t monitorInfo;
        monitorInfo.cbSize = sizeof(monitorInfo);

        // Use screen width & height if we, for some reason, can't get
        // the dimensions of the monitor
        if (!win32::GetMonitorInfo(monitor, &monitorInfo)) {
          width = win32::GetSystemMetrics(win32::SM_CXSCREEN);
          height = win32::GetSystemMetrics(win32::SM_CYSCREEN);
        } else {
          width = monitorInfo.rcMonitor.right-monitorInfo.rcMonitor.left;
          height = monitorInfo.rcMonitor.bottom-monitorInfo.rcMonitor.top;
        }

        win32::SetWindowLongPtr(win, win32::GWL_STYLE, win32::WS_POPUP);
        win32::SetWindowPos(win, NULL,
                            monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                            width, height, win32::SWP_SHOWWINDOW|win32::SWP_FRAMECHANGED);
        s_fullscreen = true;
      } else {
        win32::rect_t winSize = {};
        winSize.right = GAME_WIDTH;
        winSize.bottom = GAME_HEIGHT;

        win32::AdjustWindowRectEx(&winSize, win32::WS_OVERLAPPEDWINDOW, false, 0);

        win32::SetWindowLongPtr(win, win32::GWL_STYLE, win32::WS_OVERLAPPEDWINDOW);
        win32::SetWindowPos(win, NULL,
                            0, 0, winSize.right-winSize.left, winSize.bottom-winSize.top,
                            win32::SWP_SHOWWINDOW|win32::SWP_FRAMECHANGED|win32::SWP_NOMOVE);

        s_fullscreen = false;
      }

      return 0;
    }

    c->evt->type = KEY_EVENT;
    c->evt->key.code = VKToKey(wp, lp);

    // If this doesn't map to a key code, don't bother
    if (c->evt->key.code == KEYC_NONE) return 0;

    // If this is autorepeat, send a release event instead,
    // and queue a press event
    if (lp&0x40000000) {
      c->evt->key.flags = KEY_RELEASED_BIT|KEY_AUTOREPEAT_BIT;
      c->pressCode = c->evt->key.code;
    } else c->evt->key.flags = 0;

    c->evtFilled = true;
    return 0;

  case win32::WM_KEYUP:
  case win32::WM_SYSKEYUP:
    // Retrieve canvas pointer
    c = (win32_canvas_data_t*)win32::GetWindowLongPtr(win, win32::GWLP_USERDATA);

    c->evt->type = KEY_EVENT;
    c->evt->key.code = VKToKey(wp, lp);

    // If this doesn't map to a key code, don't bother
    if (c->evt->key.code == KEYC_NONE) return 0;

    c->evt->key.flags = KEY_RELEASED_BIT;

    c->evtFilled = true;
    return 0;
  }

  return win32::DefWindowProc(win, msg, wp, lp);
}

// FANGAME HACK, DON'T IMPORT INTO LOVEYLIB
typedef uptr dpi_awareness_context_t;
typedef b32 (WIN32_WINAPI *SetProcessDpiAwarenessContext_t)(dpi_awareness_context_t);
static constexpr const dpi_awareness_context_t DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 = -4;
static SetProcessDpiAwarenessContext_t GetDpiFunc() {
  return (SetProcessDpiAwarenessContext_t)
    win32::GetProcAddress(win32::GetModuleHandle("user32.dll"), "SetProcessDpiAwarenessContext");
}

// Create window for api-dependent canvas
static bfast CreateWin32Window(canvas_t *out, const char *title, u32 width, u32 height) {
  win32_canvas_data_t * const c = (win32_canvas_data_t*)out;

  win32::wnd_class_ex_t winClass;
  const win32::hinstance_t instance = win32::GetModuleHandle(NULL);

  // FANGAME HACK, DON'T MERGE INTO LOVEYLIB
  // I really don't care if this succeeds
  SetProcessDpiAwarenessContext_t SetProcessDpiAwarenessContext = GetDpiFunc();
  if (SetProcessDpiAwarenessContext)
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

  // Create class if it doesn't exist
  if (!win32::GetClassInfoEx(instance, S_ClassName, &winClass)) {
    winClass.size = sizeof(win32::wnd_class_ex_t);
    winClass.style = win32::CS_OWNDC|win32::CS_HREDRAW|win32::CS_VREDRAW;
    winClass.proc = (win32::wnd_proc_t)WinProc;
    winClass.clsExtra = sizeof(iptr); // Number of refs to this class
    winClass.wndExtra = 0;
    winClass.instance = instance;
    winClass.icon = win32::LoadImage(NULL, win32::MakeIntResource(win32::OIC_SAMPLE),
                                     win32::IMAGE_ICON, 0, 0,
                                     win32::LR_SHARED|win32::LR_DEFAULTSIZE);
    winClass.cursor = win32::LoadImage(NULL, win32::MakeIntResource(win32::OCR_NORMAL),
                                       win32::IMAGE_CURSOR, 0, 0,
                                       win32::LR_SHARED|win32::LR_DEFAULTSIZE);
    winClass.background = NULL;
    winClass.menuName = NULL;
    winClass.className = S_ClassName;
    winClass.iconSm = NULL;

    if (!win32::RegisterClassEx(&winClass)) return false;
  }

  // Create window
  win32::rect_t winSize = {};
  winSize.right = width;
  winSize.bottom = height;

  constexpr const u32 style = win32::WS_OVERLAPPEDWINDOW;
  win32::AdjustWindowRectEx(&winSize, style, false, 0);

  c->win = win32::CreateWindowEx(0, S_ClassName, title, style,
                                 win32::CW_USEDEFAULT,
                                 win32::CW_USEDEFAULT,
                                 winSize.right-winSize.left,
                                 winSize.bottom-winSize.top,
                                 NULL, NULL, instance, c);
  if (!c->win) return false;

  // Increment class reference count
  const iptr refCnt = win32::GetClassLongPtr(c->win, 0)+1;
  win32::SetClassLongPtr(c->win, 0, refCnt);

  // Get window device context
  c->dev = win32::GetDC(c->win);
  if (!c->dev) return false;

  // The window is now open
  c->open = true;

  return true;
}

static void CloseWin32Canvas(canvas_t *data) {
  win32_canvas_data_t * const c = (win32_canvas_data_t*)data;

  // If we have a window, close it
  if (c->win) {
    // Get class reference count
    const iptr refCnt = win32::GetClassLongPtr(c->win, 0)-1;

    if (!refCnt) {
      // If no references to this class exist, destroy it
      win32::UnregisterClass(S_ClassName, win32::GetModuleHandle(NULL));
    } else {
      // Otherwise, update reference count
      win32::SetClassLongPtr(c->win, 0, refCnt);
    }

    win32::DestroyWindow(c->win);

    // Process events for window until it's gone
    win32::msg_t msg;
    while (c->open) {
      win32::GetMessage(&msg, c->win, 0, 0);
      win32::TranslateMessage(&msg);
      win32::DispatchMessage(&msg);
    }
  }
}

// Poll win32 canvas for event
static bfast PollWin32CanvasEvent(canvas_t *data, event_t *out) {
  win32_canvas_data_t *c = (win32_canvas_data_t*)data;

  // Set output event pointer
  c->evt = out;

  // If we have to press a key for autorepeat, do so
  if (c->pressCode != KEYC_NONE) {
    c->evt->type = KEY_EVENT;
    c->evt->key.code = c->pressCode;
    c->evt->key.flags = KEY_AUTOREPEAT_BIT;
    c->pressCode = KEYC_NONE;
    return true;
  }

  // Get message, and process it in WinProc
  win32::msg_t msg;

  while (win32::PeekMessage(&msg, NULL, 0, 0, win32::PM_REMOVE)) {
    win32::TranslateMessage(&msg);
    win32::DispatchMessage(&msg);

    if (c->evtFilled) {
      c->evtFilled = false;
      return true;
    }
  }

  return false;
}

//////////////////////////////////////
// SOFTWARE RENDERER

// Win32 software canvas data
struct win32_software_canvas_data_t {
  win32_canvas_data_t g;

  win32::hbitmap_t bmp;
  win32::hdc_t bmpDev;
  win32::bitmap_info_t bmpInfo;

  u32 *scr;
};

static void RenderSoftwareCanvas(canvas_t *data) {
  win32_software_canvas_data_t *c = (win32_software_canvas_data_t*)data;
  win32::BitBlt(c->g.dev, 0, 0, c->g.pub.software.width, c->g.pub.software.height,
                c->bmpDev, 0, 0, win32::SRCCOPY);
}

static void CloseSoftwareCanvas(canvas_t *data) {
  win32_software_canvas_data_t *c = (win32_software_canvas_data_t*)data;

  // Hide window
  if (c->g.win)
    win32::ShowWindow(c->g.win, win32::SW_HIDE);

  // Destroy BMP canvas
  if (!c->bmpDev) {
    win32::DeleteDC(c->bmpDev);
    c->bmpDev = NULL;
  }

  if (!c->bmp) {
    win32::DeleteObject(c->bmp);
    c->bmp = NULL;
  }

  CloseWin32Canvas(data);
}

static const canvas_funcs_t S_SoftwareCanvasFuncs = {
  PollWin32CanvasEvent, RenderSoftwareCanvas, CloseSoftwareCanvas
};

bfast CreateSoftwareCanvas(canvas_t *out, const char *title, u32 width, u32 height) {
  out->c.base.f = &S_SoftwareCanvasFuncs;
  out->c.base.api = CANVAS_SOFTWARE_API;

  win32_software_canvas_data_t *c = (win32_software_canvas_data_t*)out;

  // Set default values
  c->g.win = c->g.dev = NULL;
  c->g.evtFilled = false;
  c->g.pressCode = KEYC_NONE;
  c->g.pub.software.surface = NULL;
  c->bmp = c->bmpDev = NULL;

  // Create base window
  if (!CreateWin32Window(out, title, width, height)) {
    CloseSoftwareCanvas(out);
    return false;
  }

  // Create window bitmap
  c->bmpInfo.hdr.size = sizeof(win32::bitmap_info_header_t);
  c->bmpInfo.hdr.width = width;
  c->bmpInfo.hdr.height = -height; // Top-down BMP
  c->bmpInfo.hdr.planes = 1;
  c->bmpInfo.hdr.bpp = 32;
  c->bmpInfo.hdr.compression = win32::BI_RGB;
  c->bmpInfo.hdr.imageSize = width*height*4;
  c->bmpInfo.hdr.hRes = 0;
  c->bmpInfo.hdr.vRes = 0;
  c->bmpInfo.hdr.palSize = 0;
  c->bmpInfo.hdr.importantPalSize = 0;

  c->bmpDev = win32::CreateCompatibleDC(c->g.dev);
  if (!c->bmpDev) {
    CloseSoftwareCanvas(out);
    return false;
  }

  c->bmp = win32::CreateDIBSection(c->g.dev, &c->bmpInfo,
                                   win32::DIB_RGB_COLORS, (void**)&c->g.pub.software.surface,
                                   NULL, 0);
  if (!c->bmp) {
    CloseSoftwareCanvas(out);
    return false;
  }

  // Bind the bitmap to this context
  if (!win32::SelectObject(c->bmpDev, c->bmp)) {
    CloseSoftwareCanvas(out);
    return false;
  }

  // Setup public parameters
  c->g.pub.software.width = c->g.pub.software.stride = width;
  c->g.pub.software.height = height;

  // Initialization complete, show window
  win32::ShowWindow(c->g.win, win32::SW_SHOW);

  return true;
}

/////////////////////////////////////////
// OpenGL renderer

#ifdef LOVEYLIB_OPENGL

// OpenGL renderer data
struct win32_gl_canvas_data_t {
  win32_canvas_data_t g;

  // OpenGL context
  win32::hglrc_t ctx;
};

// Render OpenGL canvas
static void RenderOpenGLCanvas(canvas_t *data) {
  win32_gl_canvas_data_t *c = (win32_gl_canvas_data_t*)data;

  win32::SwapBuffers(c->g.dev);
}

// Close OpenGL canvas
static void CloseOpenGLCanvas(canvas_t *data) {
  win32_gl_canvas_data_t *c = (win32_gl_canvas_data_t*)data;

  // Hide window
  if (c->g.win)
    win32::ShowWindow(c->g.win, win32::SW_HIDE);

  if (c->g.pub.gl.f.res) {
    gl::FreeFuncs(&c->g.pub.gl.f);
    c->g.pub.gl.f.res = NULL;
  }

  // Close context, if it's active
  if (c->ctx) {
    win32::wglMakeCurrent(NULL, NULL);
    win32::wglDeleteContext(c->ctx);
  }

  CloseWin32Canvas(data);
}

// OpenGL canvas vtable
static const canvas_funcs_t S_OpenGLCanvasFuncs = {
  PollWin32CanvasEvent, RenderOpenGLCanvas, CloseOpenGLCanvas
};

// Create OpenGL canvas
bfast CreateOpenGLCanvas(canvas_t *out, const char *title, u32 width, u32 height) {
  out->c.base.f = &S_OpenGLCanvasFuncs;
  out->c.base.api = CANVAS_OPENGL_API;

  win32_gl_canvas_data_t *c = (win32_gl_canvas_data_t*)out;

  // Set default values
  c->g.win = c->g.dev = NULL;
  c->g.evtFilled = false;
  c->g.pressCode = KEYC_NONE;
  c->ctx = NULL;
  c->g.pub.gl.f.res = NULL;

  // Create base window
  if (!CreateWin32Window(out, title, width, height)) {
    CloseOpenGLCanvas(out);
    return false;
  }

  // Desired pixel format
  const win32::pixel_format_descriptor_t pfd = {
    sizeof(win32::pixel_format_descriptor_t),
    1,
    win32::PFD_DRAW_TO_WINDOW|win32::PFD_SUPPORT_OPENGL| // Flags
    win32::PFD_DOUBLEBUFFER,
    win32::PFD_TYPE_RGBA, // Pixel type
    32, // Bits per pixel
    0, 0, 0, 0, 0, 0, // Bits per channel, unused
    0, // Bits per alpha channel
    0, // Alpha channel shift, unused
    0, // Accumulation buffer bits
    0, 0, 0, 0, // Accumulation buffer channel bits, unused
    24, // Depth buffer bits
    8, // Stencil buffer bits
    0, // Auxillary buffers
    win32::PFD_MAIN_PLANE, // Layer type
    0, // Unused
    0, // Unused
    0, // Unused
    0, // Unused
  };

  // Get pixel format index
  int format = win32::ChoosePixelFormat(c->g.dev, &pfd);
  if (!format) {
    CloseOpenGLCanvas(out);
    return false;
  }

  // Set pixel format
  if (!win32::SetPixelFormat(c->g.dev, format, &pfd)) {
    CloseOpenGLCanvas(out);
    return false;
  }

  // Create dummy context
  c->ctx = win32::wglCreateContext(c->g.dev);
  if (!c->ctx) {
    CloseOpenGLCanvas(out);
    return false;
  }

  // Make context current
  if (!win32::wglMakeCurrent(c->g.dev, c->ctx)) {
    CloseOpenGLCanvas(out);
    return false;
  }

  // Load extension functions
  wgl::funcs_t w;
  if (!wgl::LoadExt(&w, c->g.dev)) {
    CloseOpenGLCanvas(out);
    return false;
  }

  // Close dummy context
  win32::wglMakeCurrent(NULL, NULL);
  win32::wglDeleteContext(c->ctx);

  // New context attributes
  const int attribs[] = {
    win32::WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
    win32::WGL_CONTEXT_MINOR_VERSION_ARB, 3,
    win32::WGL_CONTEXT_PROFILE_MASK_ARB, win32::WGL_CONTEXT_CORE_PROFILE_BIT_ARB,

    0
  };

  // Create core profile context
  c->ctx = wgl::CreateContextAttribsARB(&w, c->g.dev, NULL, attribs);
  if (!c->ctx) {
    CloseOpenGLCanvas(out);
    return false;
  }

  // Make context current
  win32::wglMakeCurrent(c->g.dev, c->ctx);

  // Disable vsync, if possible
  wgl::SwapIntervalEXT(&w, 0);

  // OpenGL context is now active, load external functions
  if (!gl::LoadFuncs(&c->g.pub.gl.f)) {
    CloseOpenGLCanvas(out);
    return false;
  }
  c->g.pub.gl.f.res = (void*)1; // Mark functions as loaded

  // Initialization complete, show window
  win32::ShowWindow(c->g.win, win32::SW_SHOW);

  return true;
}

#else //LOVEYLIB_OPENGL

bfast CreateOpenGLContext(canvas_t *out, const char *title, u32 width, u32 height) {
  (void)out, (void)title, (void)width, (void)height;
  return false;
}

#endif //LOVEYLIB_OPENGL
