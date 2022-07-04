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
 * src/loveylib/xlib/loveylib_xlib_canvas.cpp:
 *  Xlib canvas interface
 *
 ************************************************************/

#include "loveylib/types.h"
#include "loveylib/canvas.h"
#include "loveylib/assert.h"
#include "loveylib/utils.h"
#include "loveylib/endian.h"
#include "loveylib_config.h"
#include "loveylib/timer.h"
#include "game.h"
#include "log.h"
#include "str.h"

// Xlib headers
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>

// XShm headers
#ifdef LOVEYLIB_XSHM

#include <X11/extensions/XShm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#endif //LOVEYLIB_XSHM

// GLX headers
#ifdef LOVEYLIB_OPENGL

#include "loveylib/opengl.h"
#include <GL/glx.h>

#endif //LOVEYLIB_OPENGL

// Global xlib data
struct xlib_canvas_data_t {
  // Public data
  canvas_data_t pub;

  Display *dis;
  Window win, root;
  int scr;
  Atom wmDeleteWindow;
  Colormap cmap;

  // Key code map
  KeySym *map;
  int minCode, maxCode, symPerCode;

  // Autorepeat data
  unsigned int lastPressed;
  key_code_t pressCode;

  // True if the window is currently mapped
  bfast winMapped;
};

// Returns class data size
static uptr MakeClassData(const char *title, char *classData) {
  u32 i;

  const uptr sizeofTitle = strlen(title)+1;

  // Fill class data
  memcpy(classData, title, sizeofTitle);
  memcpy(classData + sizeofTitle, title, sizeofTitle);

  // Filter class data
  // Instance: lowercase, no spaces
  for (i = sizeofTitle-1; i; ++classData, --i) {
    if (*classData == ' ') *classData = '-';
    else if ((u8)(*classData - 'A') < 26) *classData += 'a'-'A';
  }

  // Class: no spaces
  for (i = sizeofTitle-1, ++classData; i; ++classData, --i) {
    if (*classData == ' ') *classData = '-';
  }

  return sizeofTitle*2;
}

// Get key code from keysym
static key_code_t SymToKey(KeySym sym) {
  switch (sym) {
  case XK_Control_L: return KEYC_LCTRL;
  case XK_Control_R: return KEYC_RCTRL;
  case XK_Alt_L: return KEYC_LALT;
  case XK_Alt_R: return KEYC_RALT;
  case XK_Shift_L: return KEYC_LSHIFT;
  case XK_Shift_R: return KEYC_RSHIFT;

  case XK_Print: return KEYC_PRINTSCR;

  case XK_Pause: return KEYC_PAUSEBRK;

  case XK_BackSpace: return KEYC_BACKSPACE;
  case XK_Tab: return KEYC_TAB;
  case XK_Return: return KEYC_RETURN;

  case XK_Insert: return KEYC_INSERT;
  case XK_Begin: return KEYC_HOME;
  case XK_End: return KEYC_END;
  case XK_Page_Up: return KEYC_PAGEUP;
  case XK_Page_Down: return KEYC_PAGEDOWN;

  case XK_Up: return KEYC_UP;
  case XK_Down: return KEYC_DOWN;
  case XK_Left: return KEYC_LEFT;
  case XK_Right: return KEYC_RIGHT;

  case XK_Escape: return KEYC_ESCAPE;

  case XK_space: return KEYC_SPACE;

  case XK_apostrophe: return KEYC_APOSTROPHE;
  case XK_comma: return KEYC_COMMA;
  case XK_minus: return KEYC_HYPHEN;
  case XK_period: return KEYC_PERIOD;
  case XK_slash: return KEYC_SLASH;

  case XK_0: return KEYC_0;
  case XK_1: return KEYC_1;
  case XK_2: return KEYC_2;
  case XK_3: return KEYC_3;
  case XK_4: return KEYC_4;
  case XK_5: return KEYC_5;
  case XK_6: return KEYC_6;
  case XK_7: return KEYC_7;
  case XK_8: return KEYC_8;
  case XK_9: return KEYC_9;

  case XK_semicolon: return KEYC_SEMICOLON;
  case XK_equal: return KEYC_EQUALS;

  case XK_F1:  return KEYC_F1;
  case XK_F2:  return KEYC_F2;
  case XK_F3:  return KEYC_F3;
  case XK_F4:  return KEYC_F4;
  case XK_F5:  return KEYC_F5;
  case XK_F6:  return KEYC_F6;
  case XK_F7:  return KEYC_F7;
  case XK_F8:  return KEYC_F8;
  case XK_F9:  return KEYC_F9;
  case XK_F10: return KEYC_F10;
  case XK_F11: return KEYC_F11;
  case XK_F12: return KEYC_F12;
  case XK_F13: return KEYC_F13;
  case XK_F14: return KEYC_F14;
  case XK_F15: return KEYC_F15;
  case XK_F16: return KEYC_F16;
  case XK_F17: return KEYC_F17;
  case XK_F18: return KEYC_F18;
  case XK_F19: return KEYC_F19;
  case XK_F20: return KEYC_F20;
  case XK_F21: return KEYC_F21;
  case XK_F22: return KEYC_F22;
  case XK_F23: return KEYC_F23;
  case XK_F24: return KEYC_F24;

  case XK_bracketleft: return KEYC_OPENBRACKET;
  case XK_backslash: return KEYC_BACKSLASH;
  case XK_bracketright: return KEYC_CLOSEBRACKET;
  case XK_grave: return KEYC_GRAVE;

  case XK_a: case XK_A: return KEYC_A;
  case XK_b: case XK_B: return KEYC_B;
  case XK_c: case XK_C: return KEYC_C;
  case XK_d: case XK_D: return KEYC_D;
  case XK_e: case XK_E: return KEYC_E;
  case XK_f: case XK_F: return KEYC_F;
  case XK_g: case XK_G: return KEYC_G;
  case XK_h: case XK_H: return KEYC_H;
  case XK_i: case XK_I: return KEYC_I;
  case XK_j: case XK_J: return KEYC_J;
  case XK_k: case XK_K: return KEYC_K;
  case XK_l: case XK_L: return KEYC_L;
  case XK_m: case XK_M: return KEYC_M;
  case XK_n: case XK_N: return KEYC_N;
  case XK_o: case XK_O: return KEYC_O;
  case XK_p: case XK_P: return KEYC_P;
  case XK_q: case XK_Q: return KEYC_Q;
  case XK_r: case XK_R: return KEYC_R;
  case XK_s: case XK_S: return KEYC_S;
  case XK_t: case XK_T: return KEYC_T;
  case XK_u: case XK_U: return KEYC_U;
  case XK_v: case XK_V: return KEYC_V;
  case XK_w: case XK_W: return KEYC_W;
  case XK_x: case XK_X: return KEYC_X;
  case XK_y: case XK_Y: return KEYC_Y;
  case XK_z: case XK_Z: return KEYC_Z;

  case XK_Delete: return KEYC_DELETE;

  case XK_KP_0: return KEYC_NUM0;
  case XK_KP_1: return KEYC_NUM1;
  case XK_KP_2: return KEYC_NUM2;
  case XK_KP_3: return KEYC_NUM3;
  case XK_KP_4: return KEYC_NUM4;
  case XK_KP_5: return KEYC_NUM5;
  case XK_KP_6: return KEYC_NUM6;
  case XK_KP_7: return KEYC_NUM7;
  case XK_KP_8: return KEYC_NUM8;
  case XK_KP_9: return KEYC_NUM9;

  case XK_KP_Divide: return KEYC_NUM_DIVIDE;
  case XK_KP_Multiply: return KEYC_NUM_MULTIPLY;
  case XK_KP_Subtract: return KEYC_NUM_SUBTRACT;
  case XK_KP_Add: return KEYC_NUM_ADD;
  case XK_KP_Enter: return KEYC_NUM_ENTER;
  case XK_KP_Decimal: return KEYC_NUM_DECIMAL;

  default: return KEYC_NONE;
  }
}

// Get key code from x key code
static key_code_t CodeToKey(xlib_canvas_data_t *c, unsigned int code) {
  key_code_t ret = KEYC_NONE;
  const KeySym *k, *kEnd;

  // Find key code from x key code map
  for (kEnd = c->map + (code-c->minCode + 1)*c->symPerCode,
         k = kEnd - c->symPerCode;
       k != kEnd; ++k)
  {
    ret = SymToKey(*k);

    // If we've found one, return it
    if (ret != KEYC_NONE) return ret;
  }

  // If we've found none, return nothing
  return KEYC_NONE;
}

// FANGAME HACK, DON'T IMPORT INTO LOVEYLIB
static void ToggleFullscreen(xlib_canvas_data_t *c) {
  XEvent evt;
  evt.xclient.type = ClientMessage;
  evt.xclient.display = c->dis;
  evt.xclient.serial = 0;
  evt.xclient.send_event = True;
  evt.xclient.window = c->win;
  evt.xclient.message_type = XInternAtom(c->dis, "_NET_WM_STATE", False);
  evt.xclient.format = 32;
  evt.xclient.data.l[0] = 2;
  evt.xclient.data.l[1] = XInternAtom(c->dis, "_NET_WM_STATE_FULLSCREEN", False);
  evt.xclient.data.l[2] = 0;
  evt.xclient.data.l[3] = 0;
  evt.xclient.data.l[4] = 0;
  XSendEvent(c->dis, c->win, False, StructureNotifyMask|ResizeRedirectMask, &evt);
}

// Poll event from canvas
static bfast PollXlibCanvasEvent(canvas_t *data, event_t *out) {
  xlib_canvas_data_t *c = (xlib_canvas_data_t*)data;

  // Get event
  XEvent evt;

  // If we must repeat a key press, do so
  if (c->pressCode != KEYC_NONE) {
    out->type = KEY_EVENT;
    out->key.code = c->pressCode;
    out->key.flags = KEY_AUTOREPEAT_BIT;

    c->pressCode = KEYC_NONE;

    return true;
  }

  // Loop while event can't be converted
  for (;;) {
    if (!XPending(c->dis)) return false;
    XNextEvent(c->dis, &evt);

    // Convert it to a normal event
    switch (evt.type) {
    case ClientMessage:
      if ((Atom)evt.xclient.data.l[0] == c->wmDeleteWindow) {
        out->type = CLOSE_EVENT;
        return true;
      }

      break;

      // FANGAME HACK, DON'T IMPORT INTO LOVEYLIB!
    case ConfigureNotify:
      if (c->pub.base.api == CANVAS_OPENGL_API) {
        int x, y, width, height;
        width = evt.xconfigure.width;
        height = evt.xconfigure.height;

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
          x = (evt.xconfigure.width-width)/2;
          y = 0;
        } else {
          height = (f32)width/GAME_RATIO;
          x = 0;
          y = (evt.xconfigure.height-height)/2;
        }

        c->pub.gl.f.Viewport(x, y, width, height);
        c->pub.gl.f.Scissor(x, y, width, height);
      }
      break;

    case KeyPress:
      // FANGAME HACK, DON'T IMPORT INTO LOVEYLIB
    {
      key_code_t code = CodeToKey(c, evt.xkey.keycode);
      if ((code == KEYC_RETURN) && (evt.xkey.state&Mod1Mask)) {
        ToggleFullscreen(c);
        break;
      }

      out->type = KEY_EVENT;
      out->key.code = CodeToKey(c, evt.xkey.keycode);

      // Detect autorepeat
      out->key.flags = (evt.xkey.keycode == c->lastPressed) << KEY_AUTOREPEAT_FLAG;

      // If this wasn't a key, don't send an event for it
      if (out->key.code == KEYC_NONE) break;

      // If this key was repeated, send a release
      // event before it
      if (out->key.flags) {
        c->pressCode = out->key.code;
        out->key.flags |= KEY_RELEASED_BIT;
      }

      // This is the key that was last pressed
      c->lastPressed = evt.xkey.keycode;

      return true;
    }

    case KeyRelease:
      // If this is the last pressed key, that means it was released
      if (evt.xkey.keycode == c->lastPressed) c->lastPressed = 0;

      out->type = KEY_EVENT;
      out->key.code = CodeToKey(c, evt.xkey.keycode);
      out->key.flags = KEY_RELEASED_BIT;

      return true;
    }

    // Event can't be converted, ignore
  }
}

// Map window to screen
static inline void MapXlibWindow(xlib_canvas_data_t *c) {
  c->winMapped = true;
  XMapWindow(c->dis, c->win);
}

// Unmap window from screen
static void UnmapXlibWindow(xlib_canvas_data_t *c) {
  if (c->winMapped) {
    XUnmapWindow(c->dis, c->win);
    c->winMapped = false;
  }
}

// Create xlib window for api-dependent canvas
bfast CreateXlibWindow(xlib_canvas_data_t *c, const char *title, u32 width, u32 height, XVisualInfo *config) {
  // Enable detectable autorepeat
  Bool supported;
  XkbSetDetectableAutoRepeat(c->dis, true, &supported);
  if (!supported) return false;

  // Get keycode map
  XDisplayKeycodes(c->dis, &c->minCode, &c->maxCode);
  c->map = XGetKeyboardMapping(c->dis, c->minCode, c->maxCode-c->minCode + 1, &c->symPerCode);

  // Create colormap
  c->cmap = XCreateColormap(c->dis, c->root, config->visual, AllocNone);
  if (!c->cmap) return false;

  // Create window
  XSetWindowAttributes attribs;
  attribs.colormap = c->cmap;
  attribs.border_pixel = 0;
  attribs.event_mask = KeyPressMask|KeyReleaseMask|StructureNotifyMask;

  c->win = XCreateWindow(c->dis, c->root,
                         0, 0,
                         width, height,
                         0, config->depth,
                         InputOutput,
                         config->visual,
                         CWColormap|CWBorderPixel|CWEventMask,
                         &attribs);
  if (!c->win) return false;

  // Set window title
  const uptr titleSize = strlen(title);
  XChangeProperty(c->dis, c->win,
                  XA_WM_NAME, XA_STRING, 8,
                  PropModeReplace,
                  (const unsigned char*)title, titleSize);
  XChangeProperty(c->dis, c->win,
                  XA_WM_ICON_NAME, XA_STRING, 8,
                  PropModeReplace,
                  (const unsigned char*)title, titleSize);

  // Set window class
  char classData[128];
  const uptr classDataSize = MakeClassData(title, classData);
  XChangeProperty(c->dis, c->win,
                  XA_WM_CLASS, XA_STRING, 8,
                  PropModeReplace,
                  (const unsigned char*)classData, classDataSize);

  // Disable window resizing
//  XSizeHints size;
//  size.min_width = size.max_width = width;
//  size.min_height = size.max_height = height;
//  size.flags = PMinSize|PMaxSize;
//  XSetWMNormalHints(c->dis, c->win, &size);

  // Tell the window manager to notify us
  // when the user clicks on the X button
  c->wmDeleteWindow = XInternAtom(c->dis, "WM_DELETE_WINDOW", False);
  const Atom WM_PROTOCOLS = XInternAtom(c->dis, "WM_PROTOCOLS", False);
  XChangeProperty(c->dis, c->win,
                  WM_PROTOCOLS, XA_ATOM, 32,
                  PropModeAppend, (unsigned char*)&c->wmDeleteWindow, 1);

  return true;
}

// Close xlib canvas
static void CloseXlibCanvas(canvas_t *data) {
  xlib_canvas_data_t *c = (xlib_canvas_data_t*)data;

  // Disable detectable autorepeat
  XkbSetDetectableAutoRepeat(c->dis, false, NULL);

  // Free keycode map
  if (c->map) {
    XFree(c->map);
    c->map = NULL;
  }

  // Free colormap
  if (c->cmap != (Colormap)-1) {
    XFreeColormap(c->dis, c->cmap);
    c->cmap = (Colormap)-1;
  }

  // Close display
  if (c->dis) {
    XCloseDisplay(c->dis);
    c->dis = NULL;
  }

  // Make sure they don't try to do anything else after this
  c->pub.base.f = NULL;
}

/////////////////////////////////////////////
// SOFTWARE RENDERER

// XShm implementation of software renderer
#ifdef LOVEYLIB_XSHM

// Xlib software canvas data
struct xlib_software_canvas_data_t {
  // Global data
  xlib_canvas_data_t g;

  GC gc;
  XImage *img;
  XShmSegmentInfo segInfo;
  Status shmCompletion;
};
static_assert(sizeof(xlib_software_canvas_data_t) <= CANVAS_DATA_SIZE, "");

// Predicate function for XIfEvent
static Bool RenderEventMatches(Display *unused, XEvent *evt, XPointer shmCompletion) {
  (void)unused;

  return evt->type == (int)(uptr)shmCompletion;
}

// Render software canvas
static void RenderSoftwareCanvas(canvas_t *data) {
  xlib_software_canvas_data_t *c = (xlib_software_canvas_data_t*)data;

  // Draw the image
  XShmPutImage(c->g.dis, c->g.win, c->gc, c->img,
               0, 0, 0, 0, c->g.pub.software.width, c->g.pub.software.height, True);

  // Wait until image is done drawing
  XEvent evt;
  XIfEvent(c->g.dis, &evt, RenderEventMatches, (XPointer)(uptr)c->shmCompletion);
  ASSERT(evt.type == c->shmCompletion);
}

// Close software canvas
static void CloseSoftwareCanvas(canvas_t *data) {
  xlib_software_canvas_data_t *c = (xlib_software_canvas_data_t*)data;

  // Unmap window
  UnmapXlibWindow(&c->g);

  // If the shared memory is still allocated,
  // free it
  if (c->segInfo.shmid >= 0) {
    // Get number of attachments to memory
    struct shmid_ds shmData;
    shmctl(c->segInfo.shmid, IPC_STAT, &shmData);

    // Detach XServer from shared memory
    if (shmData.shm_nattch > 1) {
      XShmDetach(c->g.dis, &c->segInfo);
      --shmData.shm_nattch;
    }

    // Detach ourselves from shared memory
    if (shmData.shm_nattch >= 1) {
      shmdt(c->segInfo.shmaddr);
      --shmData.shm_nattch;
    }

    // It should have no attachments at this point
    ASSERT(shmData.shm_nattch == 0);

    // Remove the shared memory
    shmctl(c->segInfo.shmid, IPC_RMID, NULL);
    c->segInfo.shmid = -1;
  }

  // Destroy the window image
  if (c->img) {
    XDestroyImage(c->img);
    c->img = NULL;
  }

  // Free GC
  if (c->gc) {
    XFreeGC(c->g.dis, c->gc);
    c->gc = NULL;
  }

  CloseXlibCanvas(data);
}

// Overload
static inline void CloseSoftwareCanvas(xlib_software_canvas_data_t *c) {
  CloseSoftwareCanvas((canvas_t*)c);
}

// Software canvas vtable
static const canvas_funcs_t S_SoftwareCanvasFuncs = {
  PollXlibCanvasEvent, RenderSoftwareCanvas, CloseSoftwareCanvas
};

// Create software canvas
bfast CreateSoftwareCanvas(canvas_t *out, const char *title, u32 width, u32 height) {
  out->c.base.api = CANVAS_SOFTWARE_API;
  out->c.base.f = &S_SoftwareCanvasFuncs;

  xlib_software_canvas_data_t *c = (xlib_software_canvas_data_t*)out;

  // Setup default values
  c->g.lastPressed = 0;
  c->g.pressCode = KEYC_NONE;
  c->g.map = NULL;
  c->g.winMapped = false;
  c->segInfo.shmid = -1;
  c->img = NULL;
  c->gc = NULL;
  c->g.cmap = (Colormap)-1;
  c->g.dis = NULL;

  // Set size
  c->g.pub.software.width = width;
  c->g.pub.software.height = height;

  c->g.dis = XOpenDisplay(NULL);
  if (!c->g.dis) return false;

  // Make sure XShm is available for display
  if (!XShmQueryExtension(c->g.dis)) {
    CloseSoftwareCanvas(c);
    return false;
  }

  // Get default screen and root window of display
  c->g.scr = DefaultScreen(c->g.dis);
  c->g.root = DefaultRootWindow(c->g.dis);

  // Get shmCompletion event value
  c->shmCompletion = XShmGetEventBase(c->g.dis) + ShmCompletion;

  // Get visual infos
  XVisualInfo visualConfig = {
    NULL,
    0,
    0,
    32, // depth
    TrueColor, // class

    // Keep color in BGR_, regardless of endian
    CBIG_ENDIAN32(0x0000ff00), // red_mask
    CBIG_ENDIAN32(0x00ff0000), // green_mask
    CBIG_ENDIAN32(0xff000000), // blue_mask

    256, // colormap_size
    8, // bits_per_rgb
  };

  XVisualInfo *config;
  int configCount;
  config = XGetVisualInfo(c->g.dis,
                          VisualDepthMask|VisualClassMask|VisualRedMaskMask|
                          VisualGreenMaskMask|VisualBlueMaskMask|VisualColormapSizeMask|
                          VisualBitsPerRGBMask, &visualConfig, &configCount);
  if (!config) {
    CloseSoftwareCanvas(c);
    return false;
  }

  // Create xlib window
  if (!CreateXlibWindow((xlib_canvas_data_t*)out, title, width, height, config)) {
    CloseSoftwareCanvas(c);
    XFree(config); // Free visual config
    return false;
  }

  // Get visual from cofig, and free configs
  Visual *vis = config->visual;
  XFree(config);

  // Create GC
  XGCValues gcVals;
  c->gc = XCreateGC(c->g.dis, c->g.win, 0, &gcVals);

  // Create shared memory image
  c->img = XShmCreateImage(c->g.dis, vis,
                           32, ZPixmap,
                           NULL, &c->segInfo,
                           width, height);
  if (!c->img) {
    CloseSoftwareCanvas(c);
    return false;
  }

  // Allocate shared memory for image
  c->segInfo.shmid = shmget(IPC_PRIVATE, c->img->bytes_per_line*c->img->height, IPC_CREAT|0777);
  if (c->segInfo.shmid < 0) {
    CloseSoftwareCanvas(c);
    return false;
  }

  // Attach ourselves to the shared memory
  c->segInfo.shmaddr = c->img->data = (char*)shmat(c->segInfo.shmid, NULL, 0);
  if (c->segInfo.shmaddr == (void*)-1) {
    CloseSoftwareCanvas(c);
    return false;
  }

  c->segInfo.readOnly = False;

  // Attach shared memory to the X Server
  if (!XShmAttach(c->g.dis, &c->segInfo)) {
    CloseSoftwareCanvas(c);
    return false;
  }

  // Setup public data
  c->g.pub.software.stride = c->img->bytes_per_line/4;
  c->g.pub.software.surface = (u32*)c->segInfo.shmaddr;

  MapXlibWindow(&c->g);

  return true;
}

// TODO: No other implementation of software renderer exists!
#else //LOVEYLIB_XSHM

bfast CreateSoftwareCanvas(canvas_t *out, const char *title, u32 width, u32 height) {
  // Unused functions
  out = (canvas_t*)CloseXlibCanvas;
  out = (canvas_t*)PollXlibCanvasEvent;

  (void)out, (void)title, (void)width, (void)height;
  return false;
}

#endif //LOVEYLIB_XSHM

//////////////////////////////////////
// OPENGL RENDERER

#ifdef LOVEYLIB_OPENGL

// OpenGL canvas data
struct xlib_gl_canvas_data_t {
  xlib_canvas_data_t g;

  GLXContext ctx; // OpenGL context
  u32 width, height;
};
static_assert(sizeof(xlib_gl_canvas_data_t) <= CANVAS_DATA_SIZE, "");

// Render OpenGL canvas
static void RenderOpenGLCanvas(canvas_t *data) {
  xlib_gl_canvas_data_t *c = (xlib_gl_canvas_data_t*)data;
  glXSwapBuffers(c->g.dis, c->g.win);
}

// Close OpenGL canvas
static void CloseOpenGLCanvas(canvas_t *data) {
  xlib_gl_canvas_data_t *c = (xlib_gl_canvas_data_t*)data;

  UnmapXlibWindow(&c->g);

  // Free OpenGL functions
  if (c->g.pub.gl.f.res) {
    gl::FreeFuncs(&c->g.pub.gl.f);
    c->g.pub.gl.f.res = NULL;
  }

  // Destroy OpenGL context
  if (c->ctx) {
    glXMakeCurrent(c->g.dis, None, NULL);
    glXDestroyContext(c->g.dis, c->ctx);
    c->ctx = NULL;
  }

  CloseXlibCanvas(data);
}

static const canvas_funcs_t S_OpenGLCanvasFuncs = {
  PollXlibCanvasEvent, RenderOpenGLCanvas, CloseOpenGLCanvas
};

// Check for existence of GLX extension
static inline bptr GLXExtensionSupported(const char *extStr, const char *ext) {
  return (bptr)strstr(extStr, ext);
}

// Create OpenGL canvas
bfast CreateOpenGLCanvas(canvas_t *out, const char *title, u32 width, u32 height) {
  out->c.base.f = &S_OpenGLCanvasFuncs;
  out->c.base.api = CANVAS_OPENGL_API;

  xlib_gl_canvas_data_t *c = (xlib_gl_canvas_data_t*)out;

  // Setup default values
  c->g.lastPressed = 0;
  c->g.pressCode = KEYC_NONE;
  c->g.map = NULL;
  c->g.winMapped = false;
  c->g.cmap = (Colormap)-1;
  c->g.dis = NULL;
  c->ctx = NULL;
  c->g.pub.gl.f.res = NULL;

  // Set size
  c->g.pub.software.width = width;
  c->g.pub.software.height = height;

  c->g.dis = XOpenDisplay(NULL);
  if (!c->g.dis) return false;

  // Get default screen and root window of display
  c->g.scr = DefaultScreen(c->g.dis);
  c->g.root = DefaultRootWindow(c->g.dis);

  const int configAttribs[] = {
    GLX_DOUBLEBUFFER, True, // Double-buffered configuration

    GLX_RED_SIZE, 8, // At least 8-bits in red, green, blue, and alpha
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,

    GLX_DEPTH_SIZE, 24, // At least 24-bits in depth units
    GLX_STENCIL_SIZE, 8, // At least 8-bits in stencil units

    GLX_RENDER_TYPE, GLX_RGBA_BIT, // Can be bound to RGBA contexts
    GLX_X_RENDERABLE, True, // Has an associated x visual
    GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR, // Associated x visual is of true-color type
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT, // Is able to draw to windows
    GLX_TRANSPARENT_TYPE, GLX_NONE, // Is opaque

    None // End of list
  };

  int configCount;
  GLXFBConfig *configs = glXChooseFBConfig(c->g.dis, c->g.scr, configAttribs, &configCount);
  if (!configs) {
    CloseOpenGLCanvas(out);
    return false;
  }

  // Get visual from config
  // TODO: I may want to actually search through the list,
  //       for the best match
  XVisualInfo *vis = glXGetVisualFromFBConfig(c->g.dis, *configs);
  if (!vis) {
    CloseOpenGLCanvas(out);
    XFree(configs); // Free configs
    return false;
  }

  if (!CreateXlibWindow(&c->g, title, width, height, vis)) {
    CloseOpenGLCanvas(out);
    XFree(configs); // Free configs
    XFree(vis); // Free visual
    return false;
  }

  XFree(vis);

  // Verify we have GLX_ARB_create_context_profile
  const char *extStr = glXQueryExtensionsString(c->g.dis, c->g.scr);
  if (!GLXExtensionSupported(extStr, "GLX_ARB_create_context_profile"))
    return false;

  // Get GLX extension functions
  PFNGLXCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB =
    (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress((GLubyte*)"glXCreateContextAttribsARB");
  if (!CreateContextAttribsARB) {
    CloseOpenGLCanvas(out);
    XFree(configs); // Free configs
    return false;
  }

  // Create 3.3 GLX context
  const int contextAttribs[] = {
    GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
    GLX_CONTEXT_MINOR_VERSION_ARB, 3,
    GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
    None
  };

  c->ctx = CreateContextAttribsARB(c->g.dis, *configs, NULL, True, contextAttribs);
  if (!c->ctx) {
    CloseOpenGLCanvas(out);
    XFree(configs); // Free configs
    return false;
  }

  XFree(configs);

  // Make this the current context
  if (!glXMakeCurrent(c->g.dis, c->g.win, c->ctx)) {
    CloseOpenGLCanvas(out);
    return false;
  }

  // Load functions
  if (!gl::LoadFuncs(&c->g.pub.gl.f)) {
    CloseOpenGLCanvas(out);
    return false;
  }

  // Mark functions as loaded
  c->g.pub.gl.f.res = (void*)1;

  // Disable vsync, if possible
  if (GLXExtensionSupported(extStr, "GLX_EXT_swap_control")) {
    PFNGLXSWAPINTERVALEXTPROC SwapIntervalEXT =
      (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress((GLubyte*)"glXSwapIntervalEXT");
    if (SwapIntervalEXT)
      SwapIntervalEXT(c->g.dis, c->g.win, 0);
  }

  MapXlibWindow(&c->g);

  return true;
}

#else //LOVEYLIB_OPENGL

bfast CreateOpenGLCanvas(canvas_t *out, const char *title, u32 width, u32 height) {
  (void)out, (void)title, (void)width, (void)height;
  return false;
}

#endif //LOVEYLIB_OPENGL
