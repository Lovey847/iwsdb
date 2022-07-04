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
 * src/loveylib/win32/loveylib_windows.h:
 *  Define windows symbols in a win32 namespace
 *
 ************************************************************/

#ifndef _LOVEYLIB_WIN32_WINDOWS_H
#define _LOVEYLIB_WIN32_WINDOWS_H

#include "loveylib/types.h"
#include "loveylib_config.h"

// WINAPI, __stdcall or __attribute__((stdcall))
#if defined(LOVEYLIB_MSVC)

#define WIN32_WINAPI __stdcall

#elif defined(LOVEYLIB_GNU) //defined(LOVEYLIB_MSVC)

#define WIN32_WINAPI __attribute__((stdcall))

#else //defined(LOVEYLIB_GNU)

#error "Unknown compiler!"

#endif //defined(LOVEYLIB_MSVC)

namespace win32 {
  // Types
  typedef u32 mmresult_t;
  typedef uptr handle_t;
  typedef handle_t hwnd_t;
  typedef handle_t hdc_t;
  typedef handle_t hmenu_t;
  typedef handle_t hinstance_t;
  typedef handle_t hbitmap_t;
  typedef handle_t hicon_t;
  typedef handle_t hcursor_t;
  typedef handle_t hbrush_t;
  typedef handle_t hmodule_t;
  typedef handle_t hglrc_t;
  typedef handle_t hmonitor_t;
  typedef uptr wparam_t;
  typedef iptr lparam_t;
  typedef iptr lresult_t;
  typedef lresult_t (*wnd_proc_t)(hwnd_t /*win*/, u32 /*msg*/,
                                  wparam_t /*wp*/, lparam_t /*lp*/);
  typedef u32 (WIN32_WINAPI *thread_proc_t)(void */*param*/);
  typedef u16 atom_t;

  // Constants
  static constexpr const u32 STD_INPUT_HANDLE = (u32)-10;
  static constexpr const u32 STD_OUTPUT_HANDLE = (u32)-11;
  static constexpr const u32 STD_ERROR_HANDLE = (u32)-12;

  static constexpr const u32 FILE_BEGIN = 0;
  static constexpr const u32 FILE_CURRENT = 1;
  static constexpr const u32 FILE_END = 2;

  static constexpr const u32 CREATE_ALWAYS = 2;
  static constexpr const u32 CREATE_NEW = 1;
  static constexpr const u32 OPEN_ALWAYS = 4;
  static constexpr const u32 OPEN_EXISTING = 3;
  static constexpr const u32 TRUNCATE_EXISTING = 5;

  static constexpr const u32 GENERIC_READ = 0x80000000;
  static constexpr const u32 GENERIC_WRITE = 0x40000000;

  static constexpr const u32 FILE_ATTRIBUTE_NORMAL = 0x80;
  static constexpr const handle_t INVALID_HANDLE_VALUE = -(handle_t)1;

  static constexpr const u32 MEM_COMMIT = 0x00001000;
  static constexpr const u32 MEM_RESERVE = 0x00002000;
  static constexpr const u32 MEM_RELEASE = 0x00008000;

  static constexpr const u32 PAGE_READWRITE = 0x04;

  static constexpr const u32 VK_BACK = 0x08;
  static constexpr const u32 VK_TAB = 0x09;
  static constexpr const u32 VK_RETURN = 0x0d;
  static constexpr const u32 VK_SHIFT = 0x10;
  static constexpr const u32 VK_CONTROL = 0x11;
  static constexpr const u32 VK_MENU = 0x12;
  static constexpr const u32 VK_PAUSE = 0x13;
  static constexpr const u32 VK_ESCAPE = 0x1b;
  static constexpr const u32 VK_SPACE = 0x20;
  static constexpr const u32 VK_PRIOR = 0x21;
  static constexpr const u32 VK_NEXT = 0x22;
  static constexpr const u32 VK_END = 0x23;
  static constexpr const u32 VK_HOME = 0x24;
  static constexpr const u32 VK_LEFT = 0x25;
  static constexpr const u32 VK_UP = 0x26;
  static constexpr const u32 VK_RIGHT = 0x27;
  static constexpr const u32 VK_DOWN = 0x28;
  static constexpr const u32 VK_SNAPSHOT = 0x2c;
  static constexpr const u32 VK_INSERT = 0x2d;
  static constexpr const u32 VK_DELETE = 0x2e;
  static constexpr const u32 VK_NUMPAD0 = 0x60;
  static constexpr const u32 VK_NUMPAD1 = 0x61;
  static constexpr const u32 VK_NUMPAD2 = 0x62;
  static constexpr const u32 VK_NUMPAD3 = 0x63;
  static constexpr const u32 VK_NUMPAD4 = 0x64;
  static constexpr const u32 VK_NUMPAD5 = 0x65;
  static constexpr const u32 VK_NUMPAD6 = 0x66;
  static constexpr const u32 VK_NUMPAD7 = 0x67;
  static constexpr const u32 VK_NUMPAD8 = 0x68;
  static constexpr const u32 VK_NUMPAD9 = 0x69;
  static constexpr const u32 VK_MULTIPLY = 0x6a;
  static constexpr const u32 VK_ADD = 0x6b;
  static constexpr const u32 VK_SUBTRACT = 0x6d;
  static constexpr const u32 VK_DECIMAL = 0x6e;
  static constexpr const u32 VK_DIVIDE = 0x6f;
  static constexpr const u32 VK_F1  = 0x70;
  static constexpr const u32 VK_F2  = 0x71;
  static constexpr const u32 VK_F3  = 0x72;
  static constexpr const u32 VK_F4  = 0x73;
  static constexpr const u32 VK_F5  = 0x74;
  static constexpr const u32 VK_F6  = 0x75;
  static constexpr const u32 VK_F7  = 0x76;
  static constexpr const u32 VK_F8  = 0x77;
  static constexpr const u32 VK_F9  = 0x78;
  static constexpr const u32 VK_F10 = 0x79;
  static constexpr const u32 VK_F11 = 0x7a;
  static constexpr const u32 VK_F12 = 0x7b;
  static constexpr const u32 VK_F13 = 0x7c;
  static constexpr const u32 VK_F14 = 0x7d;
  static constexpr const u32 VK_F15 = 0x7e;
  static constexpr const u32 VK_F16 = 0x7f;
  static constexpr const u32 VK_F17 = 0x80;
  static constexpr const u32 VK_F18 = 0x81;
  static constexpr const u32 VK_F19 = 0x82;
  static constexpr const u32 VK_F20 = 0x83;
  static constexpr const u32 VK_F21 = 0x84;
  static constexpr const u32 VK_F22 = 0x85;
  static constexpr const u32 VK_F23 = 0x86;
  static constexpr const u32 VK_F24 = 0x87;
  static constexpr const u32 VK_LSHIFT = 0xa0;
  static constexpr const u32 VK_RSHIFT = 0xa1;
  static constexpr const u32 VK_LCONTROL = 0xa2;
  static constexpr const u32 VK_RCONTROL = 0xa3;
  static constexpr const u32 VK_LMENU = 0xa4;
  static constexpr const u32 VK_RMENU = 0xa5;
  static constexpr const u32 VK_OEM_1 = 0xba;
  static constexpr const u32 VK_OEM_PLUS = 0xbb;
  static constexpr const u32 VK_OEM_COMMA = 0xbc;
  static constexpr const u32 VK_OEM_MINUS = 0xbd;
  static constexpr const u32 VK_OEM_PERIOD = 0xbe;
  static constexpr const u32 VK_OEM_2 = 0xbf;
  static constexpr const u32 VK_OEM_3 = 0xc0;
  static constexpr const u32 VK_OEM_4 = 0xdb;
  static constexpr const u32 VK_OEM_5 = 0xdc;
  static constexpr const u32 VK_OEM_6 = 0xdd;
  static constexpr const u32 VK_OEM_7 = 0xde;

  static constexpr const u32 WM_NCCREATE = 0x0081;
  static constexpr const u32 WM_CLOSE = 0x0010;
  static constexpr const u32 WM_DESTROY = 0x0002;
  static constexpr const u32 WM_KEYDOWN = 0x0100;
  static constexpr const u32 WM_KEYUP = 0x0101;
  static constexpr const u32 WM_SYSKEYDOWN = 0x0104;
  static constexpr const u32 WM_SYSKEYUP = 0x0105;
  static constexpr const u32 WM_SIZE = 0x0005;

  static constexpr const int GWLP_USERDATA = -21;
  static constexpr const int GWL_STYLE = -16;

  static constexpr const u32 CS_OWNDC = 0x0020;
  static constexpr const u32 CS_HREDRAW = 0x0002;
  static constexpr const u32 CS_VREDRAW = 0x0001;

  static constexpr const u32 OCR_NORMAL = 0x7f00;
  static constexpr const u32 OCR_IBEAM = 0x7f01;
  static constexpr const u32 OIC_SAMPLE = 0x7f00;
  static constexpr const u32 OIC_QUES = 0x7f02;

  static constexpr const u32 IMAGE_CURSOR = 2;
  static constexpr const u32 IMAGE_ICON = 1;

  static constexpr const u32 LR_DEFAULTSIZE = 0x00000040;
  static constexpr const u32 LR_SHARED = 0x00008000;

  static constexpr const u32 WS_OVERLAPPED = 0x00000000;
  static constexpr const u32 WS_CAPTION = 0x00c00000;
  static constexpr const u32 WS_SYSMENU = 0x00080000;
  static constexpr const u32 WS_THICKFRAME = 0x00040000;
  static constexpr const u32 WS_MINIMIZEBOX = 0x00020000;
  static constexpr const u32 WS_MAXIMIZEBOX = 0x00010000;
  static constexpr const u32 WS_OVERLAPPEDWINDOW =
    WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|
    WS_MINIMIZEBOX|WS_MAXIMIZEBOX;
  static constexpr const u32 WS_SIZEBOX = WS_THICKFRAME;
  static constexpr const u32 WS_POPUP = 0x80000000;
  static constexpr const u32 WS_BORDER = 0x00800000;
  static constexpr const u32 WS_POPUPWINDOW =
    WS_POPUP|WS_BORDER|WS_SYSMENU;

  static constexpr const int CW_USEDEFAULT = (int)0x80000000;

  static constexpr const int SW_HIDE = 0;
  static constexpr const int SW_SHOW = 5;

  static constexpr const u32 SWP_SHOWWINDOW = 0x0040;
  static constexpr const u32 SWP_FRAMECHANGED = 0x0020;
  static constexpr const u32 SWP_NOREPOSITION = 0x0200;
  static constexpr const u32 SWP_NOMOVE = 0x0002;

  static constexpr const u32 PM_REMOVE = 0x0001;

  static constexpr const u32 MAPVK_VSC_TO_VK_EX = 3;

  static constexpr const u32 SRCCOPY = 0x00cc0020;

  static constexpr const u32 DIB_RGB_COLORS = 0;

  static constexpr const u32 BI_RGB = 0;

  static constexpr const u32 PFD_DRAW_TO_WINDOW = 0x00000004;
  static constexpr const u32 PFD_SUPPORT_OPENGL = 0x00000020;
  static constexpr const u32 PFD_DOUBLEBUFFER = 0x00000001;

  static constexpr const u8 PFD_TYPE_RGBA = 0;

  static constexpr const u8 PFD_MAIN_PLANE = 0;

  static constexpr const int SM_CXSCREEN = 0;
  static constexpr const int SM_CYSCREEN = 1;

  static constexpr const int WGL_CONTEXT_MAJOR_VERSION_ARB = 0x2091;
  static constexpr const int WGL_CONTEXT_MINOR_VERSION_ARB = 0x2092;
  static constexpr const int WGL_CONTEXT_PROFILE_MASK_ARB = 0x9126;
  static constexpr const int WGL_CONTEXT_CORE_PROFILE_BIT_ARB = 0x00000001;

  static constexpr const u32 STILL_ACTIVE = 0x103;

  static constexpr const u32 INFINITE = 0xffffffff;

  static constexpr const u32 WAIT_FAILED = 0xffffffff;
  static constexpr const u32 WAIT_OBJECT_0 = 0;

  static constexpr const u32 MONITOR_DEFAULTTONEAREST = 2;

  // Structures
  struct system_info_t {
    union {
      u32 oemId;
      u16 arch;
    };

    u32 pageSize;
    void *minAddr;
    void *maxAddr;
    u32 *activeProcessorMask;
    u32 processorCount;
    u32 processorType;
    u32 allocGranularity;
    u16 processorLevel;
    u16 processorRevision;
  };

  struct create_struct_t {
    void *createParams;
    hinstance_t instance;
    hmenu_t menu;
    hwnd_t parent;
    int height, width;
    int y, x;
    u32 style;
    const char *name;
    const char *className;
    u32 exStyle;
  };

  struct wnd_class_ex_t {
    u32 size;
    u32 style;
    wnd_proc_t proc;
    int clsExtra;
    int wndExtra;
    hinstance_t instance;
    hicon_t icon;
    hcursor_t cursor;
    hbrush_t background;
    const char *menuName;
    const char *className;
    hicon_t iconSm;
  };

  struct rect_t {
    u32 left, top, right, bottom;
  };

  struct point_t {
    u32 x, y;
  };

  struct msg_t {
    hwnd_t win;
    u32 message;
    wparam_t wp;
    lparam_t lp;
    u32 time;
    point_t pt;
    u32 priv;
  };

  struct bitmap_info_header_t {
    u32 size;
    i32 width, height;
    u16 planes, bpp;
    u32 compression;
    u32 imageSize;
    i32 hRes;
    i32 vRes;
    u32 palSize;
    u32 importantPalSize;
  };

  struct rgb_quad_t {
    u8 b, g, r, a;
  };

  struct bitmap_info_t {
    bitmap_info_header_t hdr;
    rgb_quad_t colors[1 /*hdr.paletteSize*/];
  };

  struct pixel_format_descriptor_t {
    u16 size;
    u16 version;
    u32 flags;
    u8 pixelType;
    u8 colorBits;
    u8 redBits;
    u8 redShift;
    u8 greenBits;
    u8 greenShift;
    u8 blueBits;
    u8 blueShift;
    u8 alphaBits;
    u8 alphaShift;
    u8 accumBits;
    u8 accumRedBits;
    u8 accumGreenBits;
    u8 accumBlueBits;
    u8 accumAlphaBits;
    u8 depthBits;
    u8 stencilBits;
    u8 auxBuffers;
    u8 layerType;
    u8 reserved;
    u32 layerMask;
    u32 visibleMask;
    u32 damageMask;
  };

  struct monitor_info_t {
    u32 cbSize;
    rect_t rcMonitor;
    rect_t rcWork;
    u32 dwFlags;
  };

  // Macros (defined as inline/constexpr functions where possible)
  static constexpr char *MakeIntResource(u16 id) {
    return (char*)(uptr)id;
  }

  // Wrappers
  b32 QueryPerformanceFrequency(u64 *out);
  mmresult_t TimeBeginPeriod(u32 period);
  b32 QueryPerformanceCounter(u64 *out);
  void Sleep(u32 milliseconds);
  void GetSystemInfo(system_info_t *out);
  void *VirtualAlloc(void *addr, uptr size, u32 type, u32 protect);
  b32 VirtualFree(void *addr, u32 size, u32 type);
  b32 ReadConsole(handle_t input, void *buf, u32 size, u32 *read, void *inputControl);
  b32 WriteConsole(handle_t output, const void *buf, u32 size, u32 *written, void *reserved);
  handle_t GetStdHandle(u32 handle);
  b32 ReadFile(handle_t input, void *buf, u32 size, u32 *read, void *overlapped);
  b32 WriteFile(handle_t output, const void *buf, u32 size, u32 *written, void *overlapped);
  b32 SetFilePointerEx(handle_t file, u64 dest, u64 *newPos, u32 moveMethod);
  handle_t CreateFile(const char *filename, u32 desiredAccess, u32 shareMode, void *securityAttr,
                      u32 creationDisposition, u32 flags, handle_t templateFile);
  b32 CloseHandle(handle_t handle);
  iptr SetWindowLongPtr(hwnd_t win, int index, iptr newLong);
  iptr GetWindowLongPtr(hwnd_t win, int index);
  void PostQuitMessage(int exitCode);
  lresult_t DefWindowProc(hwnd_t win, u32 msg, wparam_t wp, lparam_t lp);
  hmodule_t GetModuleHandle(const char *moduleName);
  b32 GetClassInfoEx(hinstance_t instance, const char *className, wnd_class_ex_t *out);
  handle_t LoadImage(hinstance_t instance, const char *name, u32 type,
                     int width, int height, u32 loadFlags);
  atom_t RegisterClassEx(const wnd_class_ex_t *c);
  b32 AdjustWindowRectEx(rect_t *rect, u32 style, b32 menu, u32 exStyle);
  hwnd_t CreateWindowEx(u32 exStyle, const char *className, const char *name,
                        u32 style, int x, int y, int width, int height,
                        hwnd_t parent, hmenu_t menu, hinstance_t instance,
                        void *param);
  uptr GetClassLongPtr(hwnd_t win, int index);
  uptr SetClassLongPtr(hwnd_t win, int index, iptr newLong);
  hdc_t GetDC(hwnd_t win);
  b32 ShowWindow(hwnd_t win, int cmdShow);
  b32 UnregisterClass(const char *className, hinstance_t instance);
  b32 DestroyWindow(hwnd_t win);
  b32 GetMessage(msg_t *out, hwnd_t win, u32 msgFilterMin, u32 msgFilterMax);
  b32 TranslateMessage(const msg_t *msg);
  lresult_t DispatchMessage(const msg_t *msg);
  b32 PeekMessage(msg_t *out, hwnd_t win, u32 msgFilterMin, u32 msgFilterMax, u32 removeMsg);
  u32 MapVirtualKey(u32 scanCode, u32 mapType);
  b32 BitBlt(hdc_t hdc, int x, int y, int width, int height, hdc_t src, int sx, int sy, u32 rasterOp);
  b32 DeleteDC(hdc_t hdc);
  b32 DeleteObject(handle_t obj);
  hdc_t CreateCompatibleDC(hdc_t hdc);
  hbitmap_t CreateDIBSection(hdc_t hdc, const bitmap_info_t *info, u32 usage, void **bits,
                             handle_t section, u32 offset);
  handle_t SelectObject(hdc_t hdc, handle_t h);
  int ChoosePixelFormat(hdc_t hdc, const pixel_format_descriptor_t *pfd);
  b32 SetPixelFormat(hdc_t hdc, int format, const pixel_format_descriptor_t *pfd);
  hglrc_t wglCreateContext(hdc_t hdc);
  b32 wglMakeCurrent(hdc_t hdc, hglrc_t ctx);
  void *wglGetProcAddress(const char *funcName);
  b32 wglDeleteContext(hglrc_t ctx);
  b32 SwapBuffers(hdc_t hdc);
  hmodule_t LoadLibrary(const char *libName);
  b32 FreeLibrary(hmodule_t lib);
  void *GetProcAddress(hmodule_t lib, const char *procName);
  void ExitThread(u32 exitCode);
  handle_t CreateThread(void *threadAttr, uptr stackSize, thread_proc_t threadEntryPoint, void *param,
                        u32 creationFlags, u32 *threadId);
  b32 GetExitCodeThread(handle_t thread, u32 *exitCode);
  u32 GetCurrentThreadId();
  u32 GetThreadId(handle_t thread);
  u32 WaitForSingleObject(handle_t handle, u32 milliseconds);
  handle_t CreateMutex(void *mutexAttr, b32 initialOwner, const char *name);
  b32 ReleaseMutex(handle_t mutex);
  handle_t CreateSemaphore(void *semaAttr, u32 initialCount, u32 maxCount, const char *name);
  b32 ReleaseSemaphore(handle_t semaphore, u32 releaseCount, u32 *prevCount);
  b32 SetWindowPos(hwnd_t win, hwnd_t winInsertAfter, int x, int y, int cx, int cy, u32 flags);
  int GetSystemMetrics(int index);
  hmonitor_t MonitorFromWindow(hwnd_t hwnd, u32 flags);
  b32 GetMonitorInfo(hmonitor_t monitor, monitor_info_t *out);
};

#endif //_LOVEYLIB_WIN32_WINDOWS_H
