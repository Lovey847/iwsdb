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

// RLE compression on textures
#define COMPRESS_TEXTURES

#include "loveylib/types.h"
#include "loveylib/canvas.h"
#include "loveylib/opengl.h"
#include "loveylib/vector.h"
#include "loveylib/file.h"
#include "loveylib/assert.h"
#include "loveylib_config.h"
#include "mem.h"
#include "log.h"
#include "str.h"
#include "draw.h"

#ifdef LOVEYLIB_APPLE
#define APPLE_RENDER
#include "plat/apple_render.h"
#endif

#ifndef APPLE_RENDER

#ifndef NDEBUG
#if 0
#define GLF(...) s_gl-> __VA_ARGS__; LOG_INFO(#__VA_ARGS__); if (s_gl->GetError() != gl::NO_ERROR) LOG_INFO("OpenGL error");
#else
#define GLF(...) s_gl-> __VA_ARGS__; if (s_gl->GetError() != gl::NO_ERROR) LOG_INFO("OpenGL error");
#endif
#else //NDEBUG
#define GLF(...) s_gl-> __VA_ARGS__
#endif //NDEBUG

static const char *vertexShader =
  "#version 330 core\n"
  "\n"
  "layout(location = 0) in vec3 inPos;\n"
  "layout(location = 1) in vec2 inTexCoord;\n"
  "\n"
  "noperspective out vec2 texCoord;\n"
  "\n"
  "void main() {\n"
  "  gl_Position = vec4(inPos, 1.0);\n"
  "  texCoord = inTexCoord;\n"
  "}\n";
static const char *fragmentShader =
  "#version 330 core\n"
  "\n"
  "noperspective in vec2 texCoord;\n"
  "\n"
  "uniform sampler2D tex;\n"
  "\n"
  "out vec4 fragCol;\n"
  "\n"
  "void main() {\n"
  "  fragCol = texelFetch(tex, ivec2(texCoord), 0);\n"
//  "  fragCol = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);\n"
  "  if (fragCol.a < 1.0) discard;\n"
  "}\n";
#endif  //ifndef _APPLE_RENDER_H

// Image list
// Images that don't exist in given pages are given
// {}
#define T_IMG(left, top, right, bottom, leftCoord, topCoord, rightCoord, bottomCoord) \
  {{                                                                    \
    {{{(left)*2.f/GAME_WIDTH, (top)*2.f/GAME_HEIGHT, 0.f, (leftCoord), (topCoord)}}}, \
    {{{(right)*2.f/GAME_WIDTH, (top)*2.f/GAME_HEIGHT, 0.f, (rightCoord), (topCoord)}}}, \
    {{{(left)*2.f/GAME_WIDTH, (bottom)*2.f/GAME_HEIGHT, 0.f, (leftCoord), (bottomCoord)}}}, \
    {{{(right)*2.f/GAME_WIDTH, (bottom)*2.f/GAME_HEIGHT, 0.f, (rightCoord), (bottomCoord)}}} \
    }}
#define T_IMGROT(left, top, right, bottom, leftCoord, topCoord, rightCoord, bottomCoord) \
  {{                                                                    \
    {{{(left)*2.f/GAME_WIDTH, (top)*2.f/GAME_HEIGHT, 0.f, (leftCoord), (bottomCoord)}}}, \
    {{{(right)*2.f/GAME_WIDTH, (top)*2.f/GAME_HEIGHT, 0.f, (leftCoord), (topCoord)}}}, \
    {{{(left)*2.f/GAME_WIDTH, (bottom)*2.f/GAME_HEIGHT, 0.f, (rightCoord), (bottomCoord)}}}, \
    {{{(right)*2.f/GAME_WIDTH, (bottom)*2.f/GAME_HEIGHT, 0.f, (rightCoord), (topCoord)}}} \
    }}
static constexpr const rquad_t S_Images[IMG_COUNT] = {
  // IMG_PSTAND0-3
  T_IMG(-17.f, 23.f, 32.f-17.f, -32.f+23.f, 128, 0, 160, 32),
  T_IMG(-17.f, 23.f, 32.f-17.f, -32.f+23.f, 160, 0, 192, 32),
  T_IMG(-17.f, 23.f, 32.f-17.f, -32.f+23.f, 192, 0, 224, 32),
  T_IMG(-17.f, 23.f, 32.f-17.f, -32.f+23.f, 224, 0, 256, 32),

  // IMG_PWALK0-3
  T_IMG(-17.f, 23.f, 32.f-17.f, -32.f+23.f, 128, 32, 160, 64),
  T_IMG(-17.f, 23.f, 32.f-17.f, -32.f+23.f, 160, 32, 192, 64),
  T_IMG(-17.f, 23.f, 32.f-17.f, -32.f+23.f, 192, 32, 224, 64),
  T_IMG(-17.f, 23.f, 32.f-17.f, -32.f+23.f, 224, 32, 256, 64),

  // IMG_PJUMP0-1
  T_IMG(-17.f, 23.f, 32.f-17.f, -32+23.f, 128, 64, 160, 96),
  T_IMG(-17.f, 23.f, 32.f-17.f, -32+23.f, 160, 64, 192, 96),

  // IMG_PFALL0-1
  T_IMG(-17.f, 23.f, 32.f-17.f, -32+23.f, 192, 64, 224, 96),
  T_IMG(-17.f, 23.f, 32.f-17.f, -32+23.f, 224, 64, 256, 96),

  // IMG_PVINE0-1
  T_IMG(7.f, 10.f, 7.f-21.f, -20.f+10.f, 160, 108, 181, 128),
  T_IMG(7.f, 10.f, 7.f-22.f, -20.f+10.f, 192, 108, 214, 128),

  // IMG_BULLET0-1
  T_IMG(-1.f, 1.f, 3.f, -3.f, 221, 100, 225, 104),
  T_IMG(-1.f, 1.f, 3.f, -3.f, 237, 100, 241, 104),

  // IMG_SAVE
  T_IMG(2.f, -1.f, 30.f, -32.f, 54, 130, 82, 161),

  // IMG_SAVEHIT
  T_IMG(2.f, -1.f, 30.f, -32.f, 84, 130, 112, 161),

  // IMG_WARP
  T_IMG(2.f, -2.f, 28.f, -29.f, 114, 130, 140, 157),

  // IMG_GAMEOVER
  T_IMGROT(-378.f, 79.f, 757-378.f, -158+79.f, 0, 390, 158, 1147),

  // IMG_INTRO0-5
  T_IMG(32.f, -32.f, GAME_WIDTH-32.f, -(14.f/251.f*(GAME_HEIGHT-64))-32.f, 1062, 0, 1313, 14),
  T_IMG(32.f, -32.f, GAME_WIDTH-32.f, -(28.f/232.f*(GAME_HEIGHT-64))-32.f, 1062, 16, 1062+232, 16+28),
  T_IMG(32.f, -32.f, GAME_WIDTH-32.f, -(42.f/288.f*(GAME_HEIGHT-64))-32.f, 1062, 46, 1062+288, 46+42),
  T_IMG(32.f, -32.f, GAME_WIDTH-32.f, -(42.f/209.f*(GAME_HEIGHT-64))-32.f, 1062, 90, 1062+209, 90+42),
  T_IMG(32.f, -32.f, GAME_WIDTH-32.f, -(28.f/283.f*(GAME_HEIGHT-64))-32.f, 1062, 134, 1062+283, 134+28),
  T_IMG(32.f, -32.f, GAME_WIDTH-32.f, -(42.f/238.f*(GAME_HEIGHT-64))-32.f, 1062, 164, 1062+238, 164+42),

  // IMG_JUMPSPELL
  T_IMG(4.f, -4.f, 28.f, -28.f, 134, 168, 134+24, 168+24),

  // IMG_SHOOTSPELL
  T_IMG(4.f, -4.f, 28.f, -28.f, 160, 168, 160+24, 168+24),

  // IMG_SPEEDSPELL
  T_IMG(4.f, -4.f, 28.f, -28.f, 186, 168, 186+24, 168+24),

  // IMG_FINALSPELL
  T_IMG(4.f, -4.f, 28.f, -28.f, 212, 168, 212+24, 168+24),

  // IMG_SBULLET0-1
  T_IMG(-12.f, 4.f, 4.f, -4.f, 130, 194, 130+16, 194+8),
  T_IMG(-12.f, 4.f, 4.f, -4.f, 130, 204, 130+16, 204+8),

  // IMG_SBKILLER
  T_IMG(0.f, 0.f, 32.f, -32.f, 0, 64, 32, 96),

  // IMG_DRAGON
  T_IMG(0.f, 0.f, 275.f, -334.f, 1235, 28, 1235+275, 28+334),
  // IMG_WHITEDRAGON
  T_IMG(0.f, 0.f, 275.f, -334.f, 1556, 15, 1556+275, 15+334),
  // IMG_WHITEDRAGON1
  T_IMG(0.f, 0.f, 275.f, -167.f, 1556, 15, 1556+275, 15+167),
  // IMG_WHITEDRAGON2
  T_IMG(0.f, 0.f, 275.f, -167.f, 1556, 15+167, 1556+275, 15+334),

  // IMG_THUNDER0
  T_IMG(121.f-1.f, -138.f+31.f, 541.f+121.f-1.f, -138.f-284.f+31.f, 1074, 415, 1074+541, 415+284),
  // IMG_THUNDER1
  T_IMG(120.f-1.f, -37.f+31.f, 510.f+120.f-1.f, -257.f-37.f+31.f, 1098, 714, 1098+510, 714+257),
};
#undef T_IMG
#undef T_IMGROT

#ifndef APPLE_RENDER

// Renderer state
static constexpr const uptr VBO_VERTS = 16384;
static constexpr const uptr VBO_SIZE = sizeof(vertex_t)*VBO_VERTS;

#define s_vbo s_buf[0]
#define s_ebo s_buf[1]
static gl::uint_t s_vao, s_buf[2];
static gl::uint_t s_program;
static gl::uint_t s_texture;

static gl::funcs_t *s_gl;

static vertex_t *s_vertBuf;
static uptr s_vertCount; // <= VBO_VERTS

#endif  //ifndef APPLE_RENDER

static page_t s_curPage;

#ifndef APPLE_RENDER

// Initialize shader program
static bfast InitProgram() {
  gl::uint_t vertex, fragment;

  // First, compile vertex shader
  vertex = GLF(CreateShader(gl::VERTEX_SHADER));
  if (!vertex) return false;

  GLF(ShaderSource(vertex, 1, &vertexShader, NULL));
  GLF(CompileShader(vertex));

  gl::int_t status = 0;
  GLF(GetShaderiv(vertex, gl::COMPILE_STATUS, &status));
  if (!status) {
    char errBuf[512];
    gl::sizei_t errLen;

    GLF(GetShaderInfoLog(vertex, 512, &errLen, errBuf));
    GLF(DeleteShader(vertex));

    LOG_INFO(FMT.s("Error in vertex shader: ").s(errBuf).STR);
    return false;
  }

  // Now, compile fragment shader
  fragment = GLF(CreateShader(gl::FRAGMENT_SHADER));
  if (!fragment) {
    GLF(DeleteShader(vertex));
    LOG_INFO("Couldn't create fragment shader");
    return false;
  }

  GLF(ShaderSource(fragment, 1, &fragmentShader, NULL));
  GLF(CompileShader(fragment));

  GLF(GetShaderiv(fragment, gl::COMPILE_STATUS, &status));
  if (!status) {
    char errBuf[512];
    gl::sizei_t errLen;

    GLF(GetShaderInfoLog(fragment, 512, &errLen, errBuf));
    GLF(DeleteShader(vertex));
    GLF(DeleteShader(fragment));

    LOG_INFO(FMT.s("Error in fragment shader: ").s(errBuf).STR);
    return false;
  }

  // Finally, link shader program
  s_program = GLF(CreateProgram());
  if (!s_program) {
    GLF(DeleteShader(vertex));
    GLF(DeleteShader(fragment));
    LOG_INFO("Couldn't create shader program");
    return false;
  }

  GLF(AttachShader(s_program, vertex));
  GLF(AttachShader(s_program, fragment));

  GLF(LinkProgram(s_program));

  GLF(DetachShader(s_program, vertex));
  GLF(DeleteShader(vertex));
  GLF(DetachShader(s_program, fragment));
  GLF(DeleteShader(fragment));

  GLF(GetProgramiv(s_program, gl::LINK_STATUS, &status));
  if (!status) {
    char errBuf[512];
    gl::sizei_t errLen;

    GLF(GetProgramInfoLog(s_program, 512, &errLen, errBuf));
    GLF(DeleteProgram(s_program));

    LOG_INFO(FMT.s("Link-time shader error: ").s(errBuf).STR);
    return false;
  }

  // Everything's done
  return true;
}

// Allocate & initialize local index buffer
u16 *AllocIndexBuffer() {
  u16 *ret = (u16*)Alloc((VBO_VERTS>>2)*12);

  u32 v = 0;
  for (uptr i = 0; i < (VBO_VERTS>>2)*6; i += 6, v += 4) {
    ret[i] = 0+v;
    ret[i+1] = 1+v;
    ret[i+2] = 2+v;
    ret[i+3] = 1+v;
    ret[i+4] = 2+v;
    ret[i+5] = 3+v;
  }

  return ret;
}

// Initialize OpenGL buffers
static void InitBuffers() {
  GLF(GenVertexArrays(1, &s_vao));
  GLF(BindVertexArray(s_vao));

  GLF(GenBuffers(2, s_buf));
  GLF(BindBuffer(gl::ARRAY_BUFFER, s_vbo));
  GLF(BufferData(gl::ARRAY_BUFFER, VBO_SIZE, NULL, gl::STREAM_DRAW));

  u16 *indBuf = (u16*)AllocIndexBuffer();
  GLF(BindBuffer(gl::ELEMENT_ARRAY_BUFFER, s_ebo));
  GLF(BufferData(gl::ELEMENT_ARRAY_BUFFER, (VBO_VERTS>>2)*12, indBuf, gl::STATIC_DRAW));
  Free(indBuf);

  // Setup vertex buffer attributes
  GLF(VertexAttribPointer(0, 3, gl::FLOAT, false, sizeof(vertex_t), NULL));
  GLF(VertexAttribPointer(1, 2, gl::UNSIGNED_SHORT, false, sizeof(vertex_t),
                          (void*)offsetof(vertex_t, coord.x)));
  GLF(EnableVertexAttribArray(0));
  GLF(EnableVertexAttribArray(1));
}

// Initialize texture
static void InitTexture() {
  GLF(GenTextures(1, &s_texture));
  GLF(BindTexture(gl::TEXTURE_2D, s_texture));

  // Make 2048x2048 RGBA8 texture
  GLF(TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA8, 2048, 2048, 0,
                 gl::BGRA, gl::UNSIGNED_BYTE, NULL));

  // Clamp coordinates to edge
  GLF(TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE));
  GLF(TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE));

  // No mipmap levels defined
  GLF(TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAX_LEVEL, 0));
}

// Orphan & map s_vertBuf, if required
static void OrphanVertBuf() {
  // Orphan & map buffer, if necessary
  if (!s_vertBuf) {
    GLF(BufferData(gl::ARRAY_BUFFER, VBO_SIZE, NULL, gl::STREAM_DRAW));
    s_vertBuf = (vertex_t*)GLF(MapBufferRange(gl::ARRAY_BUFFER, 0, VBO_SIZE,
                                            gl::MAP_WRITE_BIT|gl::MAP_UNSYNCHRONIZED_BIT));
  }
}

#endif  //ifndef APPLE_RENDER

// Create OpenGL window
void CreateWindow(canvas_t *out, const char *title) {
#ifndef APPLE_RENDER
  if (!CreateOpenGLCanvas(out, title, GAME_WIDTH, GAME_HEIGHT))
    LOG_ERROR("Cannot create OpenGL canvas!");

  s_gl = &out->c.gl.f;

  if (!InitProgram())
    LOG_ERROR("Cannot create shader program!");

  InitBuffers();
  InitTexture();

  GLF(UseProgram(s_program));

  // Map vertex buffer when necessary
  s_vertBuf = NULL;
  s_vertCount = 0;
#else   //ifndef APPLE_RENDER
  (void)out; (void)title;
#endif  //ifdef APPLE_RENDER

  // No page is currently active
  s_curPage = -1;

#ifndef APPLE_RENDER
  // Enable alpha blending
//  GLF(Enable(gl::BLEND));
//  GLF(BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA));

  // Enable depth test
  GLF(Enable(gl::DEPTH_TEST));
  GLF(DepthFunc(gl::LESS));
  GLF(DepthRange(0.f, 1.f));

  // Enable scissor test
  GLF(Enable(gl::SCISSOR_TEST));
#endif  //ifndef APPLE_RENDER

  // Set default clear color
  SetClearColor(0.f, 0.f, 0.f);
}

// Close OpenGL window, freeing any OpenGL resources along the way
void CloseWindow(canvas_t *c) {
#ifndef APPLE_RENDER
  GLF(DeleteTextures(1, &s_texture));
  GLF(BindBuffer(gl::ELEMENT_ARRAY_BUFFER, 0));
  GLF(BindBuffer(gl::ARRAY_BUFFER, 0));
  GLF(BindVertexArray(0));
  GLF(DeleteBuffers(2, s_buf));
  GLF(DeleteVertexArrays(1, &s_vao));
  GLF(DeleteProgram(s_program));

  CloseCanvas(c);
#else   //ifndef APPLE_RENDER
  (void)c;
#endif  //ifdef APPLE_RENDER
}

#ifndef COMPRESS_TEXTURES

// Set image page
void SetPage(page_t p) {
  if (s_curPage == p) return;
  s_curPage = p;

  // Read 2048x2048 BGRA8 image into ram, fragment write if need be
  uptr curHeight = 4096; // Divided by 2 in loop
  void *imageData;
  char filename[12] = "data/page/ ";
  filename[10] = p+'0';

  ASSERT(p < NUM_PAGES);

  // Open input page file
  stream_t f;
  f.init();
  if (!OpenFile(&f, filename, FILE_READ_ONLY))
    LOG_ERROR(FMT.s("Couldn't open \"").s(filename).s("\"!").STR);

  do {
    curHeight >>= 1;
    LOG_INFO(FMT.s("Fragmenting down to ").i(curHeight).STR);
    imageData = Alloc(2048*4*curHeight);
  } while (!imageData);

  ASSERT(curHeight > 0);

  u32 curY = 0;
  while (curY < 2048) {
    f.f->read(&f, imageData, 2048*4*curHeight);
    GLF(TexSubImage2D(gl::TEXTURE_2D, 0, 0, curY, 2048, curHeight,
                      gl::BGRA, gl::UNSIGNED_BYTE, imageData));
    curY += curHeight;
  }

  Free(imageData);
  CloseFile(&f);
}

#else //COMPRESS_TEXTURES

static inline bfast IsMarker(u32 pixel) {
  return (pixel&CBIG_ENDIAN32(0xff)) == CBIG_ENDIAN32(0x80);
}

static inline u32 MarkerLength(u32 marker) {
  return LittleEndian32(marker)&0xffffff;
}

struct decompress_state_t {
  u32 repCnt; // 0 == not repeating
  u32 curPixel;
  stream_t *file;

  #define PIXEL_CACHE_SIZE 4096
  u32 pixelCache[PIXEL_CACHE_SIZE];
  uptr pixelCacheCursor;
};

static void InitDecompression(stream_t *f, decompress_state_t *state) {
  state->repCnt = 0;
  state->file = f;
  state->pixelCacheCursor = PIXEL_CACHE_SIZE-1;
}

static u32 ReadPixel(decompress_state_t *state) {
  if (state->pixelCacheCursor < PIXEL_CACHE_SIZE-1) {
    return state->pixelCache[++state->pixelCacheCursor];
  }

  state->file->f->read(state->file, state->pixelCache, sizeof(state->pixelCache));
  state->pixelCacheCursor = 0;
  return state->pixelCache[0];
}

static u32 WritePixel(decompress_state_t *state) {
  if (state->repCnt) {
    --state->repCnt;
    return state->curPixel;
  }

  // Read pixel from file, check if it's an RLE marker
  u32 ret = ReadPixel(state);
  if (IsMarker(ret)) {
    state->repCnt = MarkerLength(ret);
    state->curPixel = ReadPixel(state);
    return state->curPixel;
  }

  return ret;
}

// Set image page
void SetPage(page_t p) {
  if (s_curPage == p) return;
  s_curPage = p;

  // Read 2048x2048 BGRA8 image into ram, fragment write if need be
  uptr curHeight = 4096; // Divided by 2 in loop
  u32 *imageData;
  char filename[13] = "data/page/ c";
  filename[10] = p+'0';

  ASSERT(p < NUM_PAGES);

  // Open input page file
  stream_t f;
  f.init();
  if (!OpenFile(&f, filename, FILE_READ_ONLY))
    LOG_ERROR(FMT.s("Couldn't open \"").s(filename).s("\"!").STR);

  // Initialize decompression state
  decompress_state_t *decomp = (decompress_state_t*)Alloc(sizeof(decompress_state_t));
  InitDecompression(&f, decomp);

  do {
    curHeight >>= 1;
    LOG_INFO(FMT.s("Fragmenting down to ").i(curHeight).STR);
    imageData = (u32*)Alloc(2048*4*curHeight);
  } while (!imageData);

  ASSERT(curHeight > 0);

  u32 curY = 0;
  while (curY < 2048) {
    for (uptr i = 0; i < 2048*curHeight; ++i)
      imageData[i] = WritePixel(decomp);

//    f.f->read(&f, imageData, 2048*4*curHeight);
#ifndef APPLE_RENDER
    GLF(TexSubImage2D(gl::TEXTURE_2D, 0, 0, curY, 2048, curHeight,
                      gl::BGRA, gl::UNSIGNED_BYTE, imageData));
#else
    AppleLoadTexturePart(imageData, 0, curY, 2048, curY + curHeight);
#endif
    curY += curHeight;
  }

  Free(imageData);
  CloseFile(&f);
}

#endif //COMPRESS_TEXTURES

// Draw image
void DrawImage(vec4 pos, vec4 scale, image_id_t img) {
  ASSERT(img < IMG_COUNT);

  // Don't draw anything if there's no active page
  if (s_curPage < 0) return;

  // Make sure parameters are as we expect them
  ASSERT(pos.v[3] == 0.f);
  ASSERT(scale.v[2] == 1.f);
  ASSERT(scale.v[3] == 1.f);

  // Setup vertices
#ifndef APPLE_RENDER
  ASSERT(s_vertCount+4 <= VBO_VERTS);

  OrphanVertBuf();
  s_vertBuf[s_vertCount].pos = pos+S_Images[img].v[0].pos*scale;
  s_vertBuf[s_vertCount+1].pos = pos+S_Images[img].v[1].pos*scale;
  s_vertBuf[s_vertCount+2].pos = pos+S_Images[img].v[2].pos*scale;
  s_vertBuf[s_vertCount+3].pos = pos+S_Images[img].v[3].pos*scale;

  s_vertCount += 4;
#else
  rquad_t quad;

  for (iptr i = 0; i < 4; ++i)
    quad.v[i].pos = S_Images[img].v[i].pos * scale + pos;

  AppleDrawQuads(&quad, 1);
#endif
}

// Draw quads
void DrawQuads(const rquad_t *quads, uptr quadCount) {
  ASSERT(quadCount > 0);

#ifndef APPLE_RENDER
  ASSERT(s_vertCount+quadCount*4 <= VBO_VERTS);

  OrphanVertBuf();
  memcpy(s_vertBuf+s_vertCount, quads, sizeof(rquad_t)*quadCount);

  s_vertCount += quadCount*4;
#else
  AppleDrawQuads(quads, quadCount);
#endif
}

// Set renderer clear color
void SetClearColor(f32 r, f32 g, f32 b) {
#ifndef APPLE_RENDER
  GLF(ClearColor(r, g, b, 1.f));
#else
  AppleSetClearColor(r, g, b);
#endif
}

// Render game state
void RenderGame() {
#ifndef APPLE_RENDER
  GLF(Clear(gl::COLOR_BUFFER_BIT|gl::DEPTH_BUFFER_BIT));

  // Unmap buffer
  if (s_vertBuf) {
    GLF(UnmapBuffer(gl::ARRAY_BUFFER));
    s_vertBuf = NULL;
  }

  // Render verts
  if (s_vertCount) {
    GLF(DrawElements(gl::TRIANGLES, (s_vertCount>>2)*6, gl::UNSIGNED_SHORT, NULL));
    s_vertCount = 0;
  }
#else
  // Handled by platform layer
#endif
}
