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
 * src/loveylib/win32/loveylib_win32_windows.cpp:
 *  Include windows and use global symbols,
 *  in an isolated environment
 *
 ************************************************************/

// INCLUDE THIS BEFORE WINDOWS.H
#include "loveylib/win32/loveylib_windows.h"
#include <windows.h>

#undef ReadConsole
#undef WriteConsole
#undef CreateFile
#undef SetWindowLongPtr
#undef GetWindowLongPtr
#undef SetClassLongPtr
#undef GetClassLongPtr
#undef CreateWindowEx
#undef RegisterClassEx
#undef UnregisterClass
#undef GetClassInfoEx
#undef GetModuleHandle
#undef LoadImage
#undef GetMessage
#undef PeekMessage
#undef DefWindowProc
#undef DispatchMessage
#undef MapVirtualKey
#undef LoadLibrary
#undef CreateMutex
#undef CreateSemaphore
#undef GetMonitorInfo

b32 win32::QueryPerformanceFrequency(u64 *out) {
  return ::QueryPerformanceFrequency((LARGE_INTEGER*)out);
}

win32::mmresult_t win32::TimeBeginPeriod(u32 period) {
  return ::timeBeginPeriod(period);
}

b32 win32::QueryPerformanceCounter(u64 *out) {
  return ::QueryPerformanceCounter((LARGE_INTEGER*)out);
}

void win32::Sleep(u32 milliseconds) {
  ::Sleep(milliseconds);
}

void win32::GetSystemInfo(system_info_t *out) {
  static_assert(sizeof(system_info_t) == sizeof(SYSTEM_INFO), "");

  ::GetSystemInfo((LPSYSTEM_INFO)out);
}

void *win32::VirtualAlloc(void *addr, uptr size, u32 type, u32 protect) {
  return ::VirtualAlloc(addr, size, type, protect);
}

b32 win32::VirtualFree(void *addr, u32 size, u32 type) {
  return ::VirtualFree(addr, size, type);
}

b32 win32::ReadConsole(win32::handle_t input, void *buf, u32 size, u32 *read, void *inputControl) {
  return ::ReadConsoleA((HANDLE)input, buf, size, (LPDWORD)read, (PCONSOLE_READCONSOLE_CONTROL)inputControl);
}

b32 win32::WriteConsole(win32::handle_t output, const void *buf, u32 size, u32 *written, void *reserved) {
  return ::WriteConsoleA((HANDLE)output, buf, size, (LPDWORD)written, reserved);
}

win32::handle_t win32::GetStdHandle(u32 handle) {
  return (win32::handle_t)::GetStdHandle(handle);
}

b32 win32::ReadFile(win32::handle_t input, void *buf, u32 size, u32 *read, void *overlapped) {
  return ::ReadFile((HANDLE)input, buf, size, (LPDWORD)read, (LPOVERLAPPED)overlapped);
}

b32 win32::WriteFile(win32::handle_t output, const void *buf, u32 size, u32 *written, void *overlapped) {
  return ::WriteFile((HANDLE)output, buf, size, (LPDWORD)written, (LPOVERLAPPED)overlapped);
}

b32 win32::SetFilePointerEx(win32::handle_t file, u64 dest, u64 *newPos, u32 moveMethod) {
  return ::SetFilePointerEx((HANDLE)file, *(LARGE_INTEGER*)&dest, (LARGE_INTEGER*)newPos, moveMethod);
}

win32::handle_t win32::CreateFile(const char *filename, u32 desiredAccess, u32 shareMode, void *securityAttr,
                                  u32 creationDisposition, u32 flags, handle_t templateFile)
{
  return (win32::handle_t)::CreateFileA(filename, desiredAccess, shareMode, (LPSECURITY_ATTRIBUTES)securityAttr,
                                        creationDisposition, flags, (HANDLE)templateFile);
}

b32 win32::CloseHandle(win32::handle_t handle) {
  return ::CloseHandle((HANDLE)handle);
}

iptr win32::SetWindowLongPtr(win32::hwnd_t win, int index, iptr newLong) {
  return ::SetWindowLongPtrA((HWND)win, index, newLong);
}

iptr win32::GetWindowLongPtr(win32::hwnd_t win, int index) {
  return ::GetWindowLongPtrA((HWND)win, index);
}

void win32::PostQuitMessage(int exitCode) {
  return ::PostQuitMessage(exitCode);
}

win32::lresult_t win32::DefWindowProc(win32::hwnd_t win, u32 msg, wparam_t wp, lparam_t lp) {
  return ::DefWindowProcA((HWND)win, msg, wp, lp);
}

win32::hmodule_t win32::GetModuleHandle(const char *moduleName) {
  return (win32::hmodule_t)::GetModuleHandleA(moduleName);
}

b32 win32::GetClassInfoEx(win32::hinstance_t instance, const char *className, win32::wnd_class_ex_t *out) {
  return ::GetClassInfoExA((HINSTANCE)instance, className, (LPWNDCLASSEXA)out);
}

win32::handle_t win32::LoadImage(win32::hinstance_t instance, const char *name, u32 type,
                                 int width, int height, u32 loadFlags)
{
  return (win32::handle_t)::LoadImageA((HINSTANCE)instance, name, type, width, height, loadFlags);
}

win32::atom_t win32::RegisterClassEx(const wnd_class_ex_t *c) {
  return ::RegisterClassExA((LPWNDCLASSEXA)c);
}

b32 win32::AdjustWindowRectEx(win32::rect_t *rect, u32 style, b32 menu, u32 exStyle) {
  return ::AdjustWindowRectEx((LPRECT)rect, style, menu, exStyle);
}

win32::hwnd_t win32::CreateWindowEx(u32 exStyle, const char *className, const char *name,
                                    u32 style, int x, int y, int width, int height,
                                    win32::hwnd_t parent, win32::hmenu_t menu,
                                    win32::hinstance_t instance, void *param)
{
  return (win32::hwnd_t)::CreateWindowExA(exStyle, className, name, style, x, y, width, height,
                           (HWND)parent, (HMENU)menu, (HINSTANCE)instance, param);
}

uptr win32::GetClassLongPtr(win32::hwnd_t win, int index) {
  return ::GetClassLongPtrA((HWND)win, index);
}

uptr win32::SetClassLongPtr(win32::hwnd_t win, int index, iptr newLong) {
  return ::SetClassLongPtrA((HWND)win, index, newLong);
}

win32::hdc_t win32::GetDC(win32::hwnd_t win) {
  return (win32::hdc_t)::GetDC((HWND)win);
}

b32 win32::ShowWindow(win32::hwnd_t win, int cmdShow) {
  return ::ShowWindow((HWND)win, cmdShow);
}

b32 win32::UnregisterClass(const char *className, win32::hinstance_t instance) {
  return ::UnregisterClassA(className, (HINSTANCE)instance);
}

b32 win32::DestroyWindow(win32::hwnd_t win) {
  return ::DestroyWindow((HWND)win);
}

b32 win32::GetMessage(win32::msg_t *out, win32::hwnd_t win, u32 msgFilterMin, u32 msgFilterMax) {
  return ::GetMessageA((LPMSG)out, (HWND)win, msgFilterMin, msgFilterMax);
}

b32 win32::TranslateMessage(const msg_t *msg) {
  return ::TranslateMessage((LPMSG)msg);
}

win32::lresult_t win32::DispatchMessage(const msg_t *msg) {
  return ::DispatchMessageA((LPMSG)msg);
}

b32 win32::PeekMessage(win32::msg_t *out, win32::hwnd_t win, u32 msgFilterMin, u32 msgFilterMax, u32 removeMsg) {
  return ::PeekMessageA((LPMSG)out, (HWND)win, msgFilterMin, msgFilterMax, removeMsg);
}

u32 win32::MapVirtualKey(u32 scanCode, u32 mapType) {
  return ::MapVirtualKeyA(scanCode, mapType);
}

b32 win32::BitBlt(win32::hdc_t hdc, int x, int y, int width, int height,
                  win32::hdc_t src, int sx, int sy, u32 rasterOp)
{
  return ::BitBlt((HDC)hdc, x, y, width, height, (HDC)src, sx, sy, rasterOp);
}

b32 win32::DeleteDC(win32::hdc_t hdc) {
  return ::DeleteDC((HDC)hdc);
}

b32 win32::DeleteObject(win32::handle_t obj) {
  return ::DeleteObject((HANDLE)obj);
}

win32::hdc_t win32::CreateCompatibleDC(win32::hdc_t hdc) {
  return (win32::hdc_t)::CreateCompatibleDC((HDC)hdc);
}

win32::hbitmap_t win32::CreateDIBSection(win32::hdc_t hdc, const win32::bitmap_info_t *info, u32 usage,
                                         void **bits, win32::handle_t section, u32 offset)
{
  return (win32::hbitmap_t)::CreateDIBSection((HDC)hdc, (LPBITMAPINFO)info, usage, bits,
                                              (HANDLE)section, offset);
}

win32::handle_t win32::SelectObject(win32::hdc_t hdc, win32::handle_t h) {
  return (win32::handle_t)::SelectObject((HDC)hdc, (HANDLE)h);
}

int win32::ChoosePixelFormat(win32::hdc_t hdc, const win32::pixel_format_descriptor_t *pfd) {
  return ::ChoosePixelFormat((HDC)hdc, (LPPIXELFORMATDESCRIPTOR)pfd);
}

b32 win32::SetPixelFormat(win32::hdc_t hdc, int format, const win32::pixel_format_descriptor_t *pfd) {
  return ::SetPixelFormat((HDC)hdc, format, (LPPIXELFORMATDESCRIPTOR)pfd);
}

win32::hglrc_t win32::wglCreateContext(win32::hdc_t hdc) {
  return (win32::hglrc_t)::wglCreateContext((HDC)hdc);
}

b32 win32::wglMakeCurrent(win32::hdc_t hdc, win32::hglrc_t ctx) {
  return ::wglMakeCurrent((HDC)hdc, (HGLRC)ctx);
}

void *win32::wglGetProcAddress(const char *funcName) {
  return (void*)::wglGetProcAddress(funcName);
}

b32 win32::wglDeleteContext(win32::hglrc_t ctx) {
  return ::wglDeleteContext((HGLRC)ctx);
}

b32 win32::SwapBuffers(win32::hdc_t hdc) {
  return ::SwapBuffers((HDC)hdc);
}

win32::hmodule_t win32::LoadLibrary(const char *libName) {
  return (win32::hmodule_t)::LoadLibraryA(libName);
}

b32 win32::FreeLibrary(win32::hmodule_t lib) {
  return ::FreeLibrary((HMODULE)lib);
}

void *win32::GetProcAddress(win32::hmodule_t lib, const char *procName) {
  return ::GetProcAddress((HMODULE)lib, procName);
}

void win32::ExitThread(u32 exitCode) {
  return ::ExitThread(exitCode);
}

win32::handle_t win32::CreateThread(void *threadAttr, uptr stackSize, thread_proc_t threadEntryPoint, void *param,
                                    u32 creationFlags, u32 *threadId)
{
  return (win32::handle_t)::CreateThread((LPSECURITY_ATTRIBUTES)threadAttr, stackSize,
                                         (LPTHREAD_START_ROUTINE)threadEntryPoint, param,
                                         creationFlags, (LPDWORD)threadId);
}

b32 win32::GetExitCodeThread(win32::handle_t thread, u32 *exitCode) {
  return ::GetExitCodeThread((HANDLE)thread, (LPDWORD)exitCode);
}

u32 win32::GetCurrentThreadId() {
  return ::GetCurrentThreadId();
}

u32 win32::GetThreadId(win32::handle_t thread) {
  return ::GetThreadId((HANDLE)thread);
}

u32 win32::WaitForSingleObject(win32::handle_t handle, u32 milliseconds) {
  return ::WaitForSingleObject((HANDLE)handle, milliseconds);
}

win32::handle_t win32::CreateMutex(void *mutexAttr, b32 initialOwner, const char *name) {
  return (win32::handle_t)::CreateMutexA((LPSECURITY_ATTRIBUTES)mutexAttr, initialOwner, name);
}

b32 win32::ReleaseMutex(win32::handle_t mutex) {
  return ::ReleaseMutex((HANDLE)mutex);
}

win32::handle_t win32::CreateSemaphore(void *semaAttr, u32 initialCount, u32 maxCount, const char *name) {
  return (win32::handle_t)::CreateSemaphoreA((LPSECURITY_ATTRIBUTES)semaAttr, initialCount, maxCount, name);
}

b32 win32::ReleaseSemaphore(win32::handle_t semaphore, u32 releaseCount, u32 *prevCount) {
  return ::ReleaseSemaphore((HANDLE)semaphore, releaseCount, (LPLONG)prevCount);
}

b32 win32::SetWindowPos(win32::hwnd_t win, win32::hwnd_t winInsertAfter, int x, int y, int cx, int cy, u32 flags) {
  return ::SetWindowPos((HWND)win, (HWND)winInsertAfter, x, y, cx, cy, flags);
}

int win32::GetSystemMetrics(int index) {
  return ::GetSystemMetrics(index);
}

win32::hmonitor_t win32::MonitorFromWindow(win32::hwnd_t hwnd, u32 flags) {
  return (win32::hmonitor_t)::MonitorFromWindow((HWND)hwnd, flags);
}

b32 win32::GetMonitorInfo(win32::hmonitor_t monitor, win32::monitor_info_t *out) {
  return ::GetMonitorInfoA((HMONITOR)monitor, (LPMONITORINFO)out);
}

// FANGAME ADDITION, DON'T MERGE INTO LOVEYLIB
int main(int argc, char **argv);
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  char *args[1] = {"fangame.exe"};
  main(1, args);
}
