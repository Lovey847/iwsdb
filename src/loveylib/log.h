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
 * src/loveylib/log.h:
 *  Information logging
 *
 ************************************************************/

#ifndef _LOVEYLIB_LOG_H
#define _LOVEYLIB_LOG_H

#include "loveylib/types.h"
#include "loveylib/stream.h"

// Maximum number of streams to log to
static constexpr const uptr MAX_LOG_STREAMS = 4;

// Log streams type, so you don't have to specify
// stream_t s[MAX_LOG_STREAMS] everytime you wanna
// log something
//
// Should be initialized to 0 when defined
// (log_streams_t s = {})
typedef stream_t log_streams_t[MAX_LOG_STREAMS];

// Open default streams
// (s STILL NEEDS TO BE ZEROED OUT)
//
// Returns false if no stream could be opened
bfast OpenDefaultLogStreams(log_streams_t s);

// Close defaut streams
// NOTE: Only use if s was opened with
// OpenDefaultLogStreams, and nothing else!
void CloseDefaultLogStreams(log_streams_t s);

// Log raw string followed by newline to
// streams in s
// If a stream in s isn't opened, it will be ignored
void LogString(log_streams_t s, const char *str);

// LogString but with (optionally automatic)
// additional information
void LogInfoExplicit(log_streams_t s, const char *file, int line, const char *str);
#define LOG_INFO(s, str) LogInfoExplicit((s), __FILE__, __LINE__, (str))

#endif //_LOVEYLIB_LOG_H
