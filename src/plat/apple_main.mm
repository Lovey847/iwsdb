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

#include "loveylib/types.h"
#include "loveylib/timer.h"
#include "loveylib/file.h"
#include "mem.h"
#include "audio.h"
#include "log.h"
#include "str.h"
#include "draw.h"
#include "game.h"
#include "plat/apple_render.h"

#include <sys/stat.h>

#import "plat/apple_view.h"

#import <Cocoa/Cocoa.h>

// Global reference to window's view
static apple_view_t *s_view;

// If false, the mainloop stops
static bfast s_running = true;

// Timer frequency, also used in apple_audio.mm
timestamp_t g_timerFrequency = 0;

// Global format string buffer used by FMT
char g_fmtStr[FMTSTR_SIZE];

// Convert timestamp to microseconds
static inline u32 TimeToMicro(timestamp_t time) {
  return time*1000000/g_timerFrequency;
}

// Move rectangle to center of screen
static void CenterRect(NSRect *rect) {
  const NSRect scrFrame = [[NSScreen mainScreen] frame];

  rect->origin.x =
    scrFrame.origin.x + (scrFrame.size.width - rect->size.width) * 0.5;
  rect->origin.y =
    scrFrame.origin.y + (scrFrame.size.height - rect->size.height) * 0.5;
}

// Create and assign app menu
static void CreateMenu() {
  NSMenu *mainMenu;
  NSMenuItem *menuItem;
  NSMenu *appMenu;

  mainMenu = [[[NSMenu alloc] initWithTitle:@"MainMenu"] autorelease];
  appMenu = [[[NSMenu alloc] initWithTitle:@"AppMenu"] autorelease];

  menuItem = [appMenu addItemWithTitle:@"About I wanna slay the dragon of "
                                        "bangan"
                                action:@selector(orderFrontStandardAboutPanel:)
                         keyEquivalent:@""];
  [menuItem setTarget:NSApp];

  [appMenu addItem:[NSMenuItem separatorItem]];

  menuItem = [appMenu addItemWithTitle:@"Quit I wanna slay the dragon of bangan"
                                action:@selector(terminate:)
                         keyEquivalent:@"q"];
  [menuItem setTarget:NSApp];

  menuItem = [mainMenu addItemWithTitle:@"I wanna slay the dragon of bangan"
                                 action:nil
                          keyEquivalent:@""];
  [mainMenu setSubmenu:appMenu forItem:menuItem];

  [NSApp setMainMenu:mainMenu];
}

// Creates a window using apple_view_t for the view, handles initialization
// and destruction of the game and all interfaces, and handles window events.
@interface iwsdb_t : NSObject<NSApplicationDelegate, NSWindowDelegate> {
  NSWindow *win;
  apple_view_t *view;
}

// s_view assigned to view in main
@property (nonatomic,weak,readwrite) apple_view_t *view;

@end    //@interface iwsdb_t

@implementation iwsdb_t

@synthesize view = view;

// Create window and its view
- (id)init {
  self = [super init];
  if (self) {
    NSRect winRect;

    winRect = NSMakeRect(0, 0, GAME_WIDTH, GAME_HEIGHT);
    CenterRect(&winRect);

    win =
      [[NSWindow alloc] initWithContentRect:winRect
                                  styleMask:(NSWindowStyleMaskTitled |
                                             NSWindowStyleMaskClosable |
                                             NSWindowStyleMaskMiniaturizable |
                                             NSWindowStyleMaskResizable)
                                    backing:NSBackingStoreBuffered
                                      defer:NO];
    [win setTitle:@"I wanna slay the dragon of bangan"];

    // iwsdb_t handles window events
    [win setDelegate:self];

    view =
      [[apple_view_t alloc] initWithFrame:NSMakeRect(0, 0, winRect.size.width,
                                                     winRect.size.height)];
    [win setContentView:view];
  }

  return self;
}

- (void)dealloc {
  [win release];
  [view release];

  [super dealloc];
}

////////////////////////////////////////////////////////////////////////////////
//      Application delegate methods

- (void)applicationWillFinishLaunching:(NSNotification*)notif {
  // Move into Resources, where the data directory resides
  const char *dataDir = [[[NSBundle mainBundle] resourcePath]
                          cStringUsingEncoding:NSUTF8StringEncoding];
  chdir(dataDir);

  // Initialize interfaces
  AllocMem();
  InitTimer();
  InitLogStreams();
  g_timerFrequency = GetTimerFrequency();

  // Error out if data directory doesn't exist
  struct stat dataDirStat;
  if (stat("data", &dataDirStat) < 0) {
    LOG_ERROR(FMT.s("Couldn't read data directory! Are you sure \"").s(dataDir)
              .s("/data\" exists?").STR);
  }

  // Error out if data isn't a directory
  if (!(dataDirStat.st_mode & S_IFDIR)) {
    LOG_ERROR(FMT.s("\"").s(dataDir).s("/data\" isn't a directory!").STR);
  }

  // On mac, this just initializes the renderer.
  CreateWindow(NULL, "I wanna slay the dragon of bangan");

  InitAudio();

  // Initialize the game
  InitGame();
}

- (void)applicationDidFinishLaunching:(NSNotification*)notif {
  CreateMenu();

  // Make widow visible
  [win makeKeyAndOrderFront:self];
}

- (void)applicationWillTerminate:(NSNotification*)notif {
  // Write save at end of execution, if possible
  stream_t saveFile = {};
  if (g_state->save.valid() &&
      OpenFile(&saveFile, "save.dat", FILE_WRITE_ONLY))
  {
    g_state->save.swap();
    saveFile.f->write(&saveFile, &g_state->save, sizeof(game_save_t));
    CloseFile(&saveFile);
  } else if (g_state->save.valid()) {
    LOG_INFO("== Unable to write save data! ==");
  }

  // Free game
  FreeGame();

  // Free interfaces
  FreeAudio();

  // On mac, this just shuts down the renderer.
  CloseWindow(NULL);

  CloseLogStreams();

  // Indicate to the mainloop that the application is done running
  s_running = false;
}

////////////////////////////////////////////////////////////////////////////////
//      Window delegate methods

- (void)windowWillClose:(NSNotification*)notif {
  [NSApp terminate:self];
}

- (void)windowDidResize:(NSNotification*)notif {
  [view UpdateRenderSize];
}

@end    //@implementation iwsdb_t

// Apple renderer methods
void AppleDrawQuads(const rquad_t *quads, uptr quadCount) {
  [s_view DrawQuads:quads
          count:quadCount];
}

void AppleLoadTexturePart(const u32 *data, u32 left, u32 top, u32 right,
                          u32 bottom)
{
  [s_view SetTexture:data
                left:left
                 top:top
               right:right
              bottom:bottom];
}

void AppleSetClearColor(f32 r, f32 g, f32 b) {
  [s_view SetClearColor:MTLClearColorMake(r, g, b, 1.f)];
}

// Apple alert methods
void AppleAlert(const char *msg) {
  NSString *str = [[[NSString alloc] initWithUTF8String:msg] autorelease];
  NSAlert *alert = [[[NSAlert alloc] init] autorelease];
  [alert setMessageText:str];
  [alert runModal];
}

int main() {
  @autoreleasepool {
    // Create NSApp
    [NSApplication sharedApplication];

    // Initialize app delegate
    iwsdb_t *app = [[[iwsdb_t alloc] init] autorelease];
    [NSApp setDelegate:app];

    // Initialize global reference to view
    s_view = app.view;

    // Request app focus
    [NSApp activate];

    // Initialize parts of the app before the mainloop
    [NSApp finishLaunching];

    do {
      // Save time of the start of the frame, in microseconds
      u32 start = TimeToMicro(GetTime());

      // Poll events
      NSEvent *evt;
      @autoreleasepool {
        for (;;) {
          evt = [NSApp nextEventMatchingMask:NSEventMaskAny
                                   untilDate:[NSDate distantPast]
                                      inMode:NSDefaultRunLoopMode
                                     dequeue:YES];
          if (!evt) break;

          [NSApp sendEvent:evt];
          [NSApp updateWindows];
        }
      }

      // Update which keys are held down in the view
      [s_view UpdateDown];
      // Update the game
      UpdateGame([s_view GetInput]);
      // Update shift key state in the view
      [s_view UpdateShift];
      // Render view's contents
      [s_view Render];
      // Update audio mixer
      UpdateAudio();

      // Delay until the time the next frame should start
      u32 end = TimeToMicro(GetTime());
      if (end - start < 1000000 / GAME_FPS) {
        MicrosecondDelay(g_timerFrequency, 1000000 / GAME_FPS - (end - start));
      }
    } while (s_running);

    // Remove reference to app.view
    s_view = nil;
  }

  return 0;
}
