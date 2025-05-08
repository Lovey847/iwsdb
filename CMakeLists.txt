#************************************************************
#*
#* Copyright (c) 2022 Lian Ferrand
#*
#* Permission is hereby granted, free of charge, to any
#* person obtaining a copy of this software and associated
#* documentation files (the "Software"), to deal in the
#* Software without restriction, including without limitation
#* the rights to use, copy, modify, merge, publish,
#* distribute, sublicense, and/or sell copies of the
#* Software, and to permit persons to whom the Software is
#* furnished to do so, subject to the following conditions:
#*
#* The above copyright notice and this permission notice
#* shall be included in all copies or substantial portions of
#* the Software.
#*
#* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
#* KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
#* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
#* PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
#* OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
#* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
#* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
#* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#*
#* This file is part of LoveyLib
#*
#* fangame/CMakeLists.txt:
#*  This file contains a CMake makefile for loveylib
#*
#************************************************************

cmake_minimum_required(VERSION 3.16.2)
project(fangame CXX)

include(TestBigEndian)

message(STATUS "${CMAKE_BUILD_TYPE}")
if (CMAKE_BUILD_TYPE STREQUAL Release)
    add_executable(fangame WIN32)
else ()
    add_executable(fangame)
endif ()

# If we're on a compiler we know, enable extra
# warnings and treat them as errors
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    if     (CMAKE_CXX_COMPILER_ID STREQUAL GNU OR
            CMAKE_CXX_COMPILER_ID STREQUAL Clang OR
            CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Werror")
    endif ()
endif ()

# Die hard release mode flags, must be explicitly enabled because
# I'm not entirely confident in their portability
option(DIE_HARD_RELEASE_BUILD OFF)
if (DIE_HARD_RELEASE_BUILD AND CMAKE_BUILD_TYPE STREQUAL Release)
    message(STATUS "Die hard release build enabled")
    message(STATUS "${CMAKE_CXX_COMPILER_ID} ${CMAKE_BUILD_TYPE}")

    if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
        # -no-pie: Disable PIE linking
        # -fno-pie: Disable PIE compilation
        # -fno-stack-protector: Disable stack smashing protector
        # -fno-mudflap: Disable mudflap pointer debugging
        # -D_FORTIFY_SOURCE=0: Disable crt security
        # -flto: Enable link-time optimization
        set(CMAKE_CXX_FLAGS_RELEASE
            "${CMAKE_CXX_FLAGS_RELEASE} \
             -O3 -no-pie -fno-pie \
             -fno-stack-protector -fno-mudflap \
             -D_FORTIFY_SOURCE=0 -flto")
    elseif ((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
        message(STATUS "stuffsy")
        set(CMAKE_CXX_FLAGS_RELEASE
            "${CMAKE_CXX_FLAGS_RELEASE} \
             /EHa- /Gw /GR- /GS- /fp:fast /fp:except- /O2 /GL")
        target_link_options(fangame PRIVATE "/SAFESEH:NO" "/INCREMENTAL:NO" "/LTCG")
        set_property(TARGET fangame PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
        message(STATUS "${CMAKE_CXX_FLAGS_RELEASE}")
    else ()
        message(STATUS "${CMAKE_CXX_COMPILER_ID} != MSVC || ${CMAKE_BUILD_TYPE} != Release")
    endif ()
endif ()

# LoveyLib information
set(LOVEYLIB_DIR "${CMAKE_SOURCE_DIR}/src")
set(LOVEYLIB_INCLUDE_DIR "${LOVEYLIB_DIR}")
set(LOVEYLIB_SOURCES
    "${LOVEYLIB_DIR}/loveylib/loveylib_mem.cpp"
    "${LOVEYLIB_DIR}/loveylib/loveylib_log.cpp"
    "${LOVEYLIB_DIR}/loveylib/loveylib_string.cpp"
    "${LOVEYLIB_DIR}/loveylib/loveylib_buffer.cpp")
set(LOVEYLIB_POSIX_SOURCES
    "${LOVEYLIB_DIR}/loveylib/posix/loveylib_posix_timer.cpp"
    "${LOVEYLIB_DIR}/loveylib/posix/loveylib_posix_heap.cpp"
    "${LOVEYLIB_DIR}/loveylib/posix/loveylib_posix_stream.cpp"
    "${LOVEYLIB_DIR}/loveylib/posix/loveylib_posix_file.cpp"
    "${LOVEYLIB_DIR}/loveylib/posix/loveylib_posix_thread.cpp")
set(LOVEYLIB_WIN32_SOURCES
    "${LOVEYLIB_DIR}/loveylib/win32/loveylib_win32_timer.cpp"
    "${LOVEYLIB_DIR}/loveylib/win32/loveylib_win32_heap.cpp"
    "${LOVEYLIB_DIR}/loveylib/win32/loveylib_win32_stream.cpp"
    "${LOVEYLIB_DIR}/loveylib/win32/loveylib_win32_file.cpp"
    "${LOVEYLIB_DIR}/loveylib/win32/loveylib_win32_canvas.cpp"
    "${LOVEYLIB_DIR}/loveylib/win32/loveylib_win32_opengl.cpp"
    "${LOVEYLIB_DIR}/loveylib/win32/loveylib_win32_thread.cpp"
    "${LOVEYLIB_DIR}/loveylib/win32/loveylib_win32_windows.cpp")
set(LOVEYLIB_XLIB_SOURCES
    "${LOVEYLIB_DIR}/loveylib/xlib/loveylib_xlib_canvas.cpp"
    "${LOVEYLIB_DIR}/loveylib/xlib/loveylib_xlib_opengl.cpp")
set(LOVEYLIB_APPLE_SOURCES
    "${LOVEYLIB_DIR}/loveylib/apple/loveylib_apple_timer.cpp"
    "${LOVEYLIB_DIR}/loveylib/posix/loveylib_posix_heap.cpp"
    "${LOVEYLIB_DIR}/loveylib/posix/loveylib_posix_stream.cpp"
    "${LOVEYLIB_DIR}/loveylib/posix/loveylib_posix_file.cpp")

# Set C++ standard
set_target_properties(fangame PROPERTIES CXX_STANDARD 11)
set_target_properties(fangame PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(fangame PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(fangame PRIVATE "${LOVEYLIB_INCLUDE_DIR}")
target_include_directories(fangame PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_include_directories(fangame PRIVATE "${CMAKE_BINARY_DIR}")
target_sources(fangame PRIVATE "${LOVEYLIB_SOURCES}")
target_sources(fangame PRIVATE
    "${CMAKE_SOURCE_DIR}/src/audio.cpp"
    "${CMAKE_SOURCE_DIR}/src/mem.cpp"
    "${CMAKE_SOURCE_DIR}/src/log.cpp"
    "${CMAKE_SOURCE_DIR}/src/game.cpp"
    "${CMAKE_SOURCE_DIR}/src/draw.cpp")

# loveylib_config.h setup
set(LOVEYLIB_OPENGL OFF)
option(LOVEYLIB_DISABLE_OPENGL "Disable OpenGL support" OFF)

# Give warning if OpenGL is explicitly disabled
if (LOVEYLIB_DISABLE_OPENGL)
    message(STATUS "NOTE: OpenGL support has been disabled")
endif ()

set(LOVEYLIB_POSIX OFF)
set(LOVEYLIB_THREADS OFF)
set(LOVEYLIB_XLIB OFF)
set(LOVEYLIB_XSHM OFF)
set(LOVEYLIB_APPLE OFF)
set(INCLUDE_MAIN_CPP ON)
if (UNIX)
    # POSIX headers
    set(LOVEYLIB_POSIX ON)

    if (APPLE)
        # Mac
        set(LOVEYLIB_APPLE ON)
        set(MAC_RESOURCES "${CMAKE_BINARY_DIR}/fangame.app/Contents/Resources")

        # Create app directory structure
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/fangame.app"
                            "${CMAKE_BINARY_DIR}/fangame.app/Contents"
                            "${CMAKE_BINARY_DIR}/fangame.app/Contents/Resources"
                            "${CMAKE_BINARY_DIR}/fangame.app/Contents/MacOS")

        # Output executable into app folder
        set_target_properties(fangame PROPERTIES OUTPUT_NAME
                              "${CMAKE_BINARY_DIR}/fangame.app/Contents/MacOS/fangame")

        # Copy Info.plist into app folder
        file(COPY_FILE "${CMAKE_SOURCE_DIR}/src/plat/apple_info.plist"
                       "${CMAKE_BINARY_DIR}/fangame.app/Contents/Info.plist")

        # Copy icon into app folder
        file(COPY_FILE "${CMAKE_SOURCE_DIR}/src/plat/apple_icon.icns"
                       "${CMAKE_BINARY_DIR}/fangame.app/Contents/Resources/icon.icns")

        # Add target for metal shaders
        set(MC_FLAGS -Wall -Wextra -pedantic -I${CMAKE_SOURCE_DIR}/src -I${CMAKE_BINARY_DIR})
        set(MC_OUT "${MAC_RESOURCES}/default.metallib")
        set(MC_SRC "${CMAKE_SOURCE_DIR}/src/plat/apple_shaders.metal")

        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(MC_FLAGS "${MC_FLAGS} -Werror -g -O0")
        elseif (CMAKE_BUIOD_TYPE STREQUAL "Release")
            set(MC_FLAGS "${MC_FLAGS} -DNDEBUG -O2")
        endif ()

        add_custom_command(OUTPUT "${MC_OUT}"
                           COMMAND xcrun -sdk macosx metal ${MC_FLAGS} ${MC_SRC} -o ${MC_OUT}
                           WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
                           COMMENT "Building metal shaders"
                           VERBATIM)
        add_custom_target(metalShaders DEPENDS "${MC_OUT}")
        add_dependencies(fangame metalShaders)

        # Enable Objective-C++ and add apple sources
        enable_language(OBJCXX)

        target_sources(fangame PRIVATE
                       "${LOVEYLIB_APPLE_SOURCES}"
                       "${CMAKE_SOURCE_DIR}/src/plat/apple_main.mm"
                       "${CMAKE_SOURCE_DIR}/src/plat/apple_view.mm"
                       "${CMAKE_SOURCE_DIR}/src/plat/apple_audio.mm")

        # Link against required frameworks
        target_link_libraries(fangame
                              "-framework Cocoa"
                              "-framework Metal"
                              "-framework Quartz"
                              "-framework AudioToolbox")

        # main is defined in apple_main.mm on mac
        set(INCLUDE_MAIN_CPP OFF)
    else ()
        #Linux
        target_sources(fangame PRIVATE "${LOVEYLIB_POSIX_SOURCES}")

        # Pthreads
        find_package(Threads)
        if (CMAKE_USE_PTHREADS_INIT)
        set(LOVEYLIB_THREADS ON)
        target_link_libraries(fangame Threads::Threads)
        endif ()

        # ALSA
        find_package(ALSA REQUIRED)
        if (NOT ALSA_FOUND)
            message(FATAL_ERROR "Cannot find ALSA!")
        endif ()
        target_sources(fangame PRIVATE "src/plat/alsa_audio.cpp")
        target_include_directories(fangame PRIVATE ${ALSA_INCLUDE_DIRS})
        target_link_libraries(fangame ALSA::ALSA)

        # Xlib
        find_package(X11)
        if (X11_FOUND)
            set(LOVEYLIB_XLIB ON)
            target_sources(fangame PRIVATE "${LOVEYLIB_XLIB_SOURCES}")
            target_include_directories(fangame PRIVATE ${X11_X11_INCLUDE_PATH})
            target_link_libraries(fangame X11::X11)

            if (X11_XShm_FOUND)
                set(LOVEYLIB_XSHM ON)
                target_include_directories(fangame PRIVATE ${X11_XShm_INCLUDE_PATH})
                target_link_libraries(fangame X11::Xext)
            endif ()

            # OpenGL
            find_package(OpenGL)
            if (OPENGL_FOUND AND OpenGL_GLX_FOUND AND NOT LOVEYLIB_DISABLE_OPENGL)
                set(LOVEYLIB_OPENGL ON)
                target_include_directories(fangame PRIVATE
                    ${OPENGL_INCLUDE_DIR}
                    ${OPENGL_GLX_INCLUDE_DIR})
                target_link_libraries(fangame OpenGL::GL OpenGL::GLX)
            endif ()
        endif ()
    endif ()
endif ()

set(LOVEYLIB_WIN32 OFF)
if (WIN32)
    set(LOVEYLIB_WIN32 ON)
    set(LOVEYLIB_OPENGL ON)
    set(LOVEYLIB_THREADS ON)
    target_sources(fangame PRIVATE "${LOVEYLIB_WIN32_SOURCES}")
    target_sources(fangame PRIVATE "${CMAKE_SOURCE_DIR}/src/plat/win32_audio.cpp")
    target_link_libraries(fangame PRIVATE winmm.lib opengl32.lib)
endif ()

TEST_BIG_ENDIAN(LOVEYLIB_BIG)
if (NOT LOVEYLIB_BIG)
    set(LOVEYLIB_LITTLE ON)
endif ()

set(LOVEYLIB_GNU OFF)
if     (CMAKE_CXX_COMPILER_ID STREQUAL GNU OR
        CMAKE_CXX_COMPILER_ID STREQUAL Clang OR
        CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
    set(LOVEYLIB_GNU ON)
endif ()

set(LOVEYLIB_MSVC OFF)
if (MSVC)
    set(LOVEYLIB_MSVC ON)
endif ()

cmake_host_system_information(RESULT FANGAME_HAS_SSE QUERY HAS_SSE)
cmake_host_system_information(RESULT FANGAME_HAS_SSE2 QUERY HAS_SSE2)

set(LOVEYLIB_SSE OFF)
if (FANGAME_HAS_SSE AND FANGAME_HAS_SSE2)
    set(LOVEYLIB_SSE ON)
endif ()

# Only include main.cpp on platforms that don't define their own entrypoint
if (INCLUDE_MAIN_CPP)
    target_sources(fangame PRIVATE "${CMAKE_SOURCE_DIR}/src/main.cpp")
endif ()

# Make loveylib_config.h
configure_file(
    "${CMAKE_SOURCE_DIR}/src/loveylib_config.h.in"
    "${CMAKE_BINARY_DIR}/loveylib_config.h")

# Print include directories, source files and libraries linked
# Could be helpful in detecting some sort of error
get_property(APP_INCLUDE_DIRECTORIES TARGET fangame PROPERTY INCLUDE_DIRECTORIES)
get_property(APP_SOURCES TARGET fangame PROPERTY SOURCES)
get_property(APP_LINK_LIBRARIES TARGET fangame PROPERTY LINK_LIBRARIES)

string(REPLACE ";" "\n     " APP_INCLUDE_DIRECTORIES "${APP_INCLUDE_DIRECTORIES}")
string(REPLACE ";" "\n     " APP_SOURCES "${APP_SOURCES}")
string(REPLACE ";" "\n     " APP_LINK_LIBRARIES "${APP_LINK_LIBRARIES}")

message(VERBOSE "Source files:\n     ${APP_SOURCES}")
message(VERBOSE "Include directories:\n     ${APP_INCLUDE_DIRECTORIES}")
message(VERBOSE "Libraries linked:\n     ${APP_LINK_LIBRARIES}")
