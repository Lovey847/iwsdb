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

#ifndef _LOG_H
#define _LOG_H

#include "loveylib/types.h"
#include "loveylib/log.h"

extern log_streams_t g_streams;

void InitLogStreams();
void CloseLogStreams();

[[noreturn]] void LogErrorExplicit(const char *file, int line, const char *str);

#undef LOG_INFO

// LOG_INFO: Log miscellaneous info
// LOG_STATUS: Log information about status, shown to end user
// LOG_ERROR: Log fatal error then quit, shown to end user

// Don't log info or show filename in release build
#ifndef NDEBUG

#define LOG_INFO(str) LogInfoExplicit(g_streams, __FILE__, __LINE__, (str))
#define LOG_STATUS(str) LOG_INFO(str)
#define LOG_ERROR(str) LogErrorExplicit(__FILE__, __LINE__, (str))

#else //NDEBUG

#define LOG_INFO(str)
#define LOG_STATUS(str) LogString(g_streams, str)
#define LOG_ERROR(str) LogErrorExplicit("", __LINE__, (str))

#endif //NDEBUG

#endif //_LOG_H
