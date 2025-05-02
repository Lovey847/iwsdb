/************************************************************
 *
 * Copyright (c) 2024 Lian Ferrand
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
 * src/loveylib/apple/loveylib_apple_canvas.mm:
 *  Apple canvas interface
 *
 ************************************************************/

#include "loveylib_config.h"
#include "loveylib/types.h"
#include "loveylib/canvas.h"
#include "loveylib/assert.h"
#include "loveylib/utils.h"
#include "loveylib/endian.h"

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Carbon/Carbon.h>

static constexpr const MTLPixelFormat PIXEL_FORMAT = MTLPixelFormatBGRA8Unorm;
static constexpr const MTLPixelFormat TEX_PIXEL_FORMAT = PIXEL_FORMAT;

@interface view_t : NSView {
  metal_canvas_t *canvas;
}

@end	//@interface view_t

@implementation view_t

- (CALayer*)makeBackingLayer {
  return [CAMetalLayer layer];
}

static id<MTLRenderPipelineState> CreatePipelineState(id<MTLDevice> device,
                                                      NSString *vertexFunc,
                                                      NSString *fragmentFunc)
{
  id<MTLLibrary> shaderLibrary = [[device newDefaultLibrary] autorelease];
  if (!shaderLibrary) return nil;

  id<MTLFunction> vertexProgram =
    [shaderLibrary newFunctionWithName:vertexFunc];
  id<MTLFunction> fragmentProgram =
    [shaderLibrary newFunctionWithName:fragmentFunc];
  if (!vertexProgram || !fragmentProgram) return nil;

  MTLRenderPipelineDescriptor *desc =
    [[[MTLRenderPipelineDescriptor alloc] init] autorelease];

  desc.label = @"Pipeline State";
  desc.vertexFunction = vertexProgram;
  desc.fragmentFunction = fragmentProgram;
  desc.colorAttachments[0].pixelFormat = PIXEL_FORMAT;
}

- (nonnull instancetype)initWithFrame:(NSRect)frame {
  self = [super initWithFrame:frame];
  if (self) {
    [self setWantsLayer:YES];
    metalLayer = (CAMetalLayer*)[self layer];

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    [metalLayer setDevice:device];
  }

  return self;
}

- (void)drawRect:(NSRect)dirtyRect {
  // Do nothing, as this is done by the user of the canvas
}

@end	//@implementation view_t

@interface window_delegate_t : NSObject<NSWindowDelegate> {
  bfast m_running;
}

- (nonnull instancetype)init;
- (bfast)isRunning;
- (void)stopRunning;

@end	//@interface window_delegate_t

@implementation window_delegate_t

- (nonnull instancetype)init {
  self = [super init];
  if (self)
    m_running = false;

  return self;
}

- (bfast)isRunning {
  return m_running;
}

- (void)stopRunning {
  m_running = false;
}

- (void)windowWillClose:(NSNotification*)notif {
  [self stopRunning];
}

@end	//@implementation window_delegate_t

struct apple_canvas_data_t {
  canvas_data_t pub;

  NSWindow *win;
};

// Returns KEYC_NONE if c doesn't correspond to any character known by loveylib
static key_code_t UnicharToKey(unichar c, NSEventModifierFlags flags) {
  if (!(flags & NSNumericKeyPad)) {
    switch (c) {
    case NSPrintScreenFunctionKey:	return KEYC_PRINTSCR;
    case NSPauseFunctionKey:		return KEYC_PAUSEBRK;
    case NSInsertFunctionKey:		return KEYC_INSERT;
    case NSF21FunctionKey:		return KEYC_F21;
    case NSF22FunctionKey:		return KEYC_F22;
    case NSF23FunctionKey:		return KEYC_F23;
    case NSF24FunctionKey:		return KEYC_F24;

    case 'A': case 'a':			return KEYC_A;
    case 'B': case 'b':			return KEYC_B;
    case 'C': case 'c':			return KEYC_C;
    case 'D': case 'd':			return KEYC_D;
    case 'E': case 'e':			return KEYC_E;
    case 'F': case 'f':			return KEYC_F;
    case 'G': case 'g':			return KEYC_G;
    case 'H': case 'h':			return KEYC_H;
    case 'I': case 'i':			return KEYC_I;
    case 'J': case 'j':			return KEYC_J;
    case 'K': case 'k':			return KEYC_K;
    case 'L': case 'l':			return KEYC_L;
    case 'M': case 'm':			return KEYC_M;
    case 'N': case 'n':			return KEYC_N;
    case 'O': case 'o':			return KEYC_O;
    case 'P': case 'p':			return KEYC_P;
    case 'Q': case 'q':			return KEYC_Q;
    case 'R': case 'r':			return KEYC_R;
    case 'S': case 's':			return KEYC_S;
    case 'T': case 't':			return KEYC_T;
    case 'U': case 'u':			return KEYC_U;
    case 'V': case 'v':			return KEYC_V;
    case 'W': case 'w':			return KEYC_W;
    case 'X': case 'x':			return KEYC_X;
    case 'Y': case 'y':			return KEYC_Y;
    case 'Z': case 'z':			return KEYC_Z;

    case '0': case ')':			return KEYC_0;
    case '1': case '!':			return KEYC_1;
    case '2': case '@':			return KEYC_2;
    case '3': case '#':			return KEYC_3;
    case '4': case '$':			return KEYC_4;
    case '5': case '%':			return KEYC_5;
    case '6': case '^':			return KEYC_6;
    case '7': case '&':			return KEYC_7;
    case '8': case '*':			return KEYC_8;
    case '9': case '(':			return KEYC_9;

    case '-': case '_':			return KEYC_HYPHEN;
    case '=': case '+':			return KEYC_EQUALS;
    case '\'': case '"':		return KEYC_APOSTROPHE;
    case ',': case '<':			return KEYC_COMMA;
    case '.': case '>':			return KEYC_PERIOD;
    case '/': case '?':			return KEYC_SLASH;
    case ';': case ':':			return KEYC_SEMICOLON;
    case '[': case '{':			return KEYC_OPENBRACKET;
    case ']': case '}':			return KEYC_CLOSEBRACKET;
    case '\\': case '|':		return KEYC_BACKSLASH;
    case '`': case '~':			return KEYC_GRAVE;
    }
  } else {
    switch (c) {
    case '0':				return KEYC_NUM0;
    case '1':				return KEYC_NUM1;
    case '2':				return KEYC_NUM2;
    case '3':				return KEYC_NUM3;
    case '4':				return KEYC_NUM4;
    case '5':				return KEYC_NUM5;
    case '6':				return KEYC_NUM6;
    case '7':				return KEYC_NUM7;
    case '8':				return KEYC_NUM8;
    case '9':				return KEYC_NUM9;

    case '/':				return KEYC_NUM_DIVIDE;
    case '*':				return KEYC_NUM_MULTIPLY;
    case '-':				return KEYC_NUM_SUBTRACT;
    case '+':				return KEYC_NUM_ADD;
    case '\r':				return KEYC_NUM_ENTER;
    case '.':				return KEYC_NUM_DECIMAL;
    }
  }

  return KEYC_NONE;
}

// Returns KEYC_NONE if c doesn't correspond to any character known by loveylib
static key_code_t KeyCodeToKey(unsigned short c) {
  switch (c) {
  case kVK_Control:			return KEYC_LCTRL;
  case kVK_RightControl:		return KEYC_RCTRL;
  case kVK_Option:			return KEYC_LALT;
  case kVK_RightOption:			return KEYC_RALT;
  case kVK_Shift:			return KEYC_LSHIFT;
  case kVK_RightShift:			return KEYC_RSHIFT;
  case kVK_Return:			return KEYC_RETURN;
  case kVK_Home:			return KEYC_HOME;
  case kVK_End:				return KEYC_END;
  case kVK_PageUp:			return KEYC_PAGEUP;
  case kVK_PageDown:			return KEYC_PAGEDOWN;
  case kVK_UpArrow:			return KEYC_UP;
  case kVK_DownArrow:			return KEYC_DOWN;
  case kVK_LeftArrow:			return KEYC_LEFT;
  case kVK_RightArrow:			return KEYC_RIGHT;
  case kVK_Escape:			return KEYC_ESCAPE;
  case kVK_Space:			return KEYC_SPACE;
  case kVK_F1:				return KEYC_F1;
  case kVK_F2:				return KEYC_F2;
  case kVK_F3:				return KEYC_F3;
  case kVK_F4:				return KEYC_F4;
  case kVK_F5:				return KEYC_F5;
  case kVK_F6:				return KEYC_F6;
  case kVK_F7:				return KEYC_F7;
  case kVK_F8:				return KEYC_F8;
  case kVK_F9:				return KEYC_F9;
  case kVK_F10:				return KEYC_F10;
  case kVK_F11:				return KEYC_F11;
  case kVK_F12:				return KEYC_F12;
  case kVK_F13:				return KEYC_F13;
  case kVK_F14:				return KEYC_F14;
  case kVK_F15:				return KEYC_F15;
  case kVK_F16:				return KEYC_F16;
  case kVK_F17:				return KEYC_F17;
  case kVK_F18:				return KEYC_F18;
  case kVK_F19:				return KEYC_F19;
  case kVK_F20:				return KEYC_F20;
  case kVK_ForwardDelete:		return KEYC_BACKSPACE;
  case kVK_Delete:			return KEYC_DELETE;
  case kVK_Tab:				return KEYC_TAB;

  default:				return KEYC_NONE;
  }
}

static key_code_t StringToKey(NSString *str, NSEventModifierFlags flags) {
  if ([str length] > 1) return KEYC_NONE;
  return UnicharToKey([str characterAtIndex:0], flags);
}

static key_code_t EventToKey(NSEvent *ev) {
  key_code_t ret = KeyCodeToKey([ev keyCode]);
  if (ret == KEYC_NONE)
    ret = StringToKey([ev charactersIgnoringModifiers], [ev modifierFlags]);

  return ret;
}

static bfast ConvertEvent(NSEvent *ev, event_t *output) {
  switch ([ev type]) {
  case NSEventTypeKeyDown:
  case NSEventTypeKeyUp:
    output->type = KEY_EVENT;
    output->key.code = EventToKey(ev);
    if (output->key.code == KEYC_NONE) return false;

    output->key.flags = 0;
    if ([ev type] == NSEventTypeKeyUp) output->key.flags |= KEY_RELEASED_BIT;
    if ([ev isARepeat]) output->key.flags |= KEY_AUTOREPEAT_BIT;
    return true;

  default:
    return false;
  }
}

static void CloseAppleCanvas(canvas_t *data) {
  apple_canvas_data_t * const c = (apple_canvas_data_t*)data;

  if (c->win) [c->win close];
}

static bfast PollAppleCanvasEvent(canvas_t *data, event_t *output) {
  @autoreleasepool {
    NSEvent *ev;
    do {
      ev = [NSApp nextEventMatchingMask:NSAnyEventMask
			      untilDate:nil
				 inMode:NSDefaultRunLoopMode
				dequeue:YES];

      if (ev) {
	if (ConvertEvent(ev, output)) return true;
      }
    } while (ev);
  }

  return false;
}

static void RenderDummy(canvas_t *data) {
  (void)data;

  // Do nothing
}

// No software canvas implementation yet
bfast CreateSoftwareCanvas(canvas_t *output, const char *title, u32 width,
			   u32 height)
{
  (void)output;
  (void)title;
  (void)width;
  (void)height;

  return false;
}

// No opengl canvas implementation yet
bfast CreateOpenGLCanvas(canvas_t *output, const char *title, u32 width,
			 u32 height)
{
  (void)output;
  (void)title;
  (void)width;
  (void)height;

  return false;
}

bfast CreateMetalCanvas(canvas_t *output, const char *title, u32 width,
			u32 height)
{
  output->c.base.f = &S_MetalCanvasFuncs;
}
