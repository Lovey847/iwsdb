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
 * src/loveylib/loveylib_key_codes.h:
 *  Keyboard key code list
 *
 ************************************************************/

#ifndef _LOVEYLIB_LOVEYLIB_KEY_CODES_H
#define _LOVEYLIB_LOVEYLIB_KEY_CODES_H

#include "loveylib/types.h"

// Key codes, these map to keyboard keys on a
// US ANSI keyboard. Maybe it'll be extended in
// the future
//
// Not all of ASCII maps to keyboard codes, only
// common escape codes, unshifted symbols, numbers,
// and the lowercase alphabet
//
// This enum type is subject to grow from ufast
// in the future
enum key_code_e : ufast {
  // ASCII
  // (with very uncommon codes replaced
  //  by more common keyboard keys)
  KEYC_LCTRL = 0, KEYC_RCTRL,
  KEYC_LALT, KEYC_RALT,
  KEYC_LSHIFT, KEYC_RSHIFT,

  KEYC_PRINTSCR,
  KEYC_PAUSEBRK,

  KEYC_BACKSPACE = 0x08, // '\b'
  KEYC_TAB = 0x09, // '\t'
  KEYC_RETURN = 0x0a, // '\n'

  KEYC_INSERT,
  KEYC_HOME,
  KEYC_END,
  KEYC_PAGEUP,
  KEYC_PAGEDOWN,

  KEYC_UP = 0x12,
  KEYC_DOWN,
  KEYC_LEFT,
  KEYC_RIGHT,

  KEYC_ESCAPE = 0x1b,

  KEYC_SPACE = 0x20, // ' '

  KEYC_APOSTROPHE = 0x27, // '\''
  KEYC_COMMA = 0x2c, // ','
  KEYC_HYPHEN = 0x2d, // '-'
  KEYC_PERIOD = 0x2e, // '.'
  KEYC_SLASH = 0x2f, // '/'

  KEYC_0 = 0x30, // '0'
  KEYC_1 = 0x31, // '1'
  KEYC_2 = 0x32, // '2'
  KEYC_3 = 0x33, // '3'
  KEYC_4 = 0x34, // '4'
  KEYC_5 = 0x35, // '5'
  KEYC_6 = 0x36, // '6'
  KEYC_7 = 0x37, // '7'
  KEYC_8 = 0x38, // '8'
  KEYC_9 = 0x39, // '9'

  KEYC_SEMICOLON = 0x3b, // ';'
  KEYC_EQUALS = 0x3d, // '='

  KEYC_F1,  KEYC_F2,  KEYC_F3,  KEYC_F4,
  KEYC_F5,  KEYC_F6,  KEYC_F7,  KEYC_F8,
  KEYC_F9,  KEYC_F10, KEYC_F11, KEYC_F12,
  KEYC_F13, KEYC_F14, KEYC_F15, KEYC_F16,
  KEYC_F17, KEYC_F18, KEYC_F19, KEYC_F20,
  KEYC_F21, KEYC_F22, KEYC_F23, KEYC_F24,

  KEYC_OPENBRACKET = 0x5b, // '['
  KEYC_BACKSLASH = 0x5c, // '\\'
  KEYC_CLOSEBRACKET = 0x5d, // ']'
  KEYC_GRAVE = 0x60, // '`'

  KEYC_A = 0x61, // 'a'
  KEYC_B = 0x62, // 'b'
  KEYC_C = 0x63, // 'c'
  KEYC_D = 0x64, // 'd'
  KEYC_E = 0x65, // 'e'
  KEYC_F = 0x66, // 'f'
  KEYC_G = 0x67, // 'g'
  KEYC_H = 0x68, // 'h'
  KEYC_I = 0x69, // 'i'
  KEYC_J = 0x6a, // 'j'
  KEYC_K = 0x6b, // 'k'
  KEYC_L = 0x6c, // 'l'
  KEYC_M = 0x6d, // 'm'
  KEYC_N = 0x6e, // 'n'
  KEYC_O = 0x6f, // 'o'
  KEYC_P = 0x70, // 'p'
  KEYC_Q = 0x71, // 'q'
  KEYC_R = 0x72, // 'r'
  KEYC_S = 0x73, // 's'
  KEYC_T = 0x74, // 't'
  KEYC_U = 0x75, // 'u'
  KEYC_V = 0x76, // 'v'
  KEYC_W = 0x77, // 'w'
  KEYC_X = 0x78, // 'x'
  KEYC_Y = 0x79, // 'y'
  KEYC_Z = 0x7a, // 'z'

  KEYC_DELETE = 0x7f,

  // Numpad
  KEYC_NUM0, KEYC_NUM1,
  KEYC_NUM2, KEYC_NUM3,
  KEYC_NUM4, KEYC_NUM5,
  KEYC_NUM6, KEYC_NUM7,
  KEYC_NUM8, KEYC_NUM9,

  KEYC_NUM_DIVIDE,
  KEYC_NUM_MULTIPLY,
  KEYC_NUM_SUBTRACT,
  KEYC_NUM_ADD,
  KEYC_NUM_ENTER,
  KEYC_NUM_DECIMAL,

  KEYC_NONE = 0xff
};
typedef ufast key_code_t;

#endif //_LOVEYLIB_LOVEYLIB_KEY_CODES_H
