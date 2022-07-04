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
#include "loveylib/canvas.h"
#include "loveylib/file.h"
#include "mem.h"
#include "audio.h"
#include "log.h"
#include "str.h"
#include "draw.h"
#include "game.h"

timestamp_t g_timerFrequency = 0;

static inline u32 TimeToMicro(timestamp_t time) {
  return time*1000000/g_timerFrequency;
}

// From str.h
char g_fmtStr[FMTSTR_SIZE];

int main(int argc, char **argv) {
  // For now, unused
  (void)argc, (void)argv;

  AllocMem();
  InitTimer();
  InitLogStreams();

  g_timerFrequency = GetTimerFrequency();

  canvas_t win;
  input_t input = {};

  CreateWindow(&win, "I wanna slay the dragon of bangan");
  InitAudio();

  InitGame();

  event_t evt;

  // lshift and rshift are broken on windows
#ifndef LOVEYLIB_WIN32
  bfast pressShiftNextFrame, releaseShiftNextFrame, lshiftDown, rshiftDown;
  pressShiftNextFrame = releaseShiftNextFrame = lshiftDown = rshiftDown = false;
  bfast lshiftPressed, rshiftPressed;
  bfast lshiftReleased, rshiftReleased;
  lshiftPressed = lshiftReleased =
    rshiftPressed = rshiftReleased = false;
#else
  bfast pressShiftNextFrame, releaseShiftNextFrame;
  pressShiftNextFrame = releaseShiftNextFrame = false;
  bfast lshiftPressed, rshiftPressed;
  bfast lshiftReleased, rshiftReleased;
  lshiftPressed = lshiftReleased =
    rshiftPressed = rshiftReleased = false;
#endif
  for (;;) {
    const u32 start = TimeToMicro(GetTime());

    while (PollCanvasEvent(&win, &evt)) {
      input_field_t *dest;

      switch (evt.type) {
      case CLOSE_EVENT: goto l_end;
      case KEY_EVENT:
        if (evt.key.flags&KEY_AUTOREPEAT_BIT) break;

        dest = &input.pressed + (evt.key.flags&KEY_RELEASED_BIT);

        switch (evt.key.code) {
        case KEYC_ESCAPE: goto l_end;

        case KEYC_LEFT: *dest |= INPUT_LEFTBIT; break;
        case KEYC_RIGHT: *dest |= INPUT_RIGHTBIT; break;
        case KEYC_UP: *dest |= INPUT_UPBIT; break;
        case KEYC_DOWN: *dest |= INPUT_DOWNBIT; break;

#ifndef LOVEYLIB_WIN32
        case KEYC_LSHIFT:
          if (dest == &input.pressed) {
            if (lshiftReleased) pressShiftNextFrame = true;
            else if (!rshiftDown) *dest |= INPUT_JUMPBIT;
            lshiftDown = true;
            lshiftPressed = true;
          } else {
            if (lshiftPressed) releaseShiftNextFrame = true;
            else if (!rshiftDown) *dest |= INPUT_JUMPBIT;
            lshiftDown = false;
            lshiftReleased = true;
          }
          break;
        case KEYC_RSHIFT:
          if (dest == &input.pressed) {
            if (rshiftReleased) pressShiftNextFrame = true;
            else if (!lshiftDown) *dest |= INPUT_JUMPBIT;
            rshiftDown = true;
            rshiftPressed = true;
          } else {
            if (rshiftPressed) releaseShiftNextFrame = true;
            else if (!lshiftDown) *dest |= INPUT_JUMPBIT;
            rshiftDown = false;
            rshiftReleased = true;
          }
          break;
#else
        case KEYC_LSHIFT:
          if (dest == &input.pressed) {
            if (lshiftReleased) pressShiftNextFrame = true;
            else *dest |= INPUT_JUMPBIT;
            lshiftPressed = true;
          } else {
            if (lshiftPressed) releaseShiftNextFrame = true;
            else *dest |= INPUT_JUMPBIT;
            lshiftReleased = true;
          }
          break;
        case KEYC_RSHIFT:
          if (dest == &input.pressed) {
            if (rshiftReleased) pressShiftNextFrame = true;
            else *dest |= INPUT_JUMPBIT;
            rshiftPressed = true;
          } else {
            if (rshiftPressed) releaseShiftNextFrame = true;
            else *dest |= INPUT_JUMPBIT;
            rshiftReleased = true;
          }
          break;
#endif

        case KEYC_Z: *dest |= INPUT_SHOOTBIT; break;
        case KEYC_R: *dest |= INPUT_RESTARTBIT; break;
        case KEYC_F2: *dest |= INPUT_NEWGAMEBIT; break;
        }
      }
    }

    input.updateDown();

    UpdateGame(&input);

    // Only reset these if input was processed this frame
    if (!input.pressed) {
      if (pressShiftNextFrame) input.pressed |= INPUT_JUMPBIT;
      if (releaseShiftNextFrame) input.released |= INPUT_JUMPBIT;

      pressShiftNextFrame = releaseShiftNextFrame = false;
      lshiftPressed = lshiftReleased =
        rshiftPressed = rshiftReleased = false;
    }
    RenderCanvas(&win);
    UpdateAudio();

    const u32 end = TimeToMicro(GetTime());
    if (end-start < 1000000/GAME_FPS)
      MicrosecondDelay(g_timerFrequency, 1000000/GAME_FPS - (end-start));
  }

l_end:
  // Before we shut down, write the game save to a file
  // Only write save data if it's valid
  stream_t saveFile = {};
  if (g_state->save.valid() && OpenFile(&saveFile, "save.dat", FILE_WRITE_ONLY)) {
    g_state->save.swap();
    saveFile.f->write(&saveFile, &g_state->save, sizeof(game_save_t));
    CloseFile(&saveFile);
  } else if (g_state->save.valid()) LOG_STATUS("== Unable to write save data! ==");

  FreeGame();
  CloseWindow(&win);
  FreeAudio();
  CloseLogStreams();

  return 0;
}
