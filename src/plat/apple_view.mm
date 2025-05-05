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

#import "plat/apple_view.h"

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include "loveylib/types.h"
#include "log.h"
#include "str.h"
#include "game.h"

#define PIXEL_FORMAT MTLPixelFormatBGRA8Unorm
#define TEX_PIXEL_FORMAT MTLPixelFormatBGRA8Unorm
#define DEPTH_PIXEL_FORMAT MTLPixelFormatDepth32Float

#define PI 3.141596535897932384626f

#define MAX_QUADS 4096

#define MODFLAG_SHIFT 0x20000
#define MODFLAG_SHIFTPOS 17
#define MODFLAG_LEFT 2
#define MODFLAG_RIGHT 4

// Texture coordinate of clear color
#define CLRCOL_COORD_X 2047
#define CLRCOL_COORD_Y 2047

struct input_state_t {
  input_t input;

  bfast pressShiftNextFrame, releaseShiftNextFrame;
  bfast lshiftDown, lshiftPressed, lshiftReleased;
  bfast rshiftDown, rshiftPressed, rshiftReleased;

  ifast lastModifierMask;
};

static u16 *AllocIndexBuffer();
static id<MTLRenderPipelineState> CreatePipelineState(id<MTLDevice> device,
                                                      NSString *vertexFunc,
                                                      NSString *fragmentFunc);
static id<MTLTexture> CreateTexture(id<MTLDevice> device);
static id<MTLDepthStencilState> CreateDepthState(id<MTLDevice> device);
static id<MTLTexture> CreateDepthBuf(id<MTLDevice> device,
                                     NSUInteger width, NSUInteger height);
static void RenderView(apple_view_t *view);
static void ProcessFlagsChanged(input_state_t *state, NSEvent *evt);
static input_field_t KeyCodeToField(u16 keyCode);
static void SetClearColor(id<MTLTexture> texture, f32 r, f32 g, f32 b);
static void CalcLetterBox(apple_viewport_t *viewport, NSUInteger width,
                          NSUInteger height, f32 targetRatio);

static constexpr const rquad_t S_ClearQuad = {{
    {{{-1.f, 1.f, 0.9999999f, CLRCOL_COORD_X, CLRCOL_COORD_Y}}},
    {{{1.f, 1.f, 0.9999999f, CLRCOL_COORD_X + 1, CLRCOL_COORD_Y}}},
    {{{-1.f, -1.f, 0.9999999f, CLRCOL_COORD_X, CLRCOL_COORD_Y + 1}}},
    {{{1.f, -1.f, 0.9999999f, CLRCOL_COORD_X + 1, CLRCOL_COORD_Y + 1}}}
  }};

@implementation apple_view_t {
  id<MTLTexture> depthBuf;
  input_state_t input;

  f32 clearColR, clearColG, clearColB;

  apple_viewport_t viewport;
}

@synthesize cmdQueue = cmdQueue;
@synthesize metalLayer = metalLayer;
@synthesize renderDescriptor = renderDescriptor;
@synthesize pipelineState = pipelineState;
@synthesize depthState = depthState;
@synthesize vertices = vertices;
@synthesize texture = texture;
@synthesize quadCount = quadCount;
@synthesize indices = indices;

- (nonnull instancetype)initWithFrame:(NSRect)frame {
  self = [super initWithFrame:frame];
  if (self) {
    metalLayer = [CAMetalLayer layer];
    [self setLayer:metalLayer];
    [self setWantsLayer:YES];

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    [metalLayer setDevice:device];

    [metalLayer setPixelFormat:PIXEL_FORMAT];

    cmdQueue = [device newCommandQueue];

    depthBuf = CreateDepthBuf(device, (NSUInteger)frame.size.width,
                              (NSUInteger)frame.size.height);
    depthState = CreateDepthState(device);

    renderDescriptor = [MTLRenderPassDescriptor new];
    renderDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    renderDescriptor.colorAttachments[0].clearColor =
      MTLClearColorMake(0, 0, 0, 1);

    renderDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
    renderDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
    renderDescriptor.depthAttachment.clearDepth = 1.f;
    renderDescriptor.depthAttachment.texture = depthBuf;

    pipelineState = CreatePipelineState(device, @"VertexShader",
                                        @"FragmentShader");

    vertices = [device newBufferWithLength:(MAX_QUADS * sizeof(rquad_t))
                                   options:MTLResourceStorageModeShared];
    if (!vertices) LOG_ERROR("Couldn't create vertex buffer!");

    u16 *indexBufferContents = AllocIndexBuffer();
    indices =
      [device newBufferWithBytes:indexBufferContents
                          length:(MAX_QUADS * 6 * sizeof(u16))
                         options:(MTLResourceStorageModeShared |
                                  MTLResourceHazardTrackingModeUntracked)];
    if (!indices) LOG_ERROR("Couldn't create index buffer!");
    free(indexBufferContents);

    texture = CreateTexture(device);

    quadCount = 0;

    clearColR = clearColG = clearColB = 0.f;
    SetClearColor(texture, 0.f, 0.f, 0.f);

    CalcLetterBox(&viewport, frame.size.width, frame.size.height,
                  (f32)GAME_WIDTH / (f32)GAME_HEIGHT);
  }

  return self;
}

- (void)dealloc {
  [super dealloc];
}

- (void)UpdateRenderSize {
  id<MTLDevice> device = MTLCreateSystemDefaultDevice();

  // Create new metal layer
  metalLayer = [CAMetalLayer layer];
  [self setLayer:metalLayer];
  [metalLayer setDevice:device];

  // Resize depth buffer
  NSUInteger width = (NSUInteger)[super frame].size.width;
  NSUInteger height = (NSUInteger)[super frame].size.height;
  depthBuf = CreateDepthBuf(device, width, height);
  renderDescriptor.depthAttachment.texture = depthBuf;

  // Recalculate viewport
  CalcLetterBox(&viewport, width, height,
                (f32)GAME_WIDTH / (f32)GAME_HEIGHT);
}

- (void)DrawQuads:(const rquad_t*)quads count:(iptr)count {
  if (quadCount + count >= MAX_QUADS) count = MAX_QUADS - quadCount;
  if (count <= 0) return;

  rquad_t *out = (rquad_t*)[vertices contents] + quadCount;

  // Arrange the vertices where v[0] is the top left, v[1] is the top
  // right, v[2] is the bottom left, and v[3] is the bottom right of the image
  // in the texture page.
  // For an explanation of why this is done, see CoordOffsets in
  // plat/apple_shaders.metal.
  // TODO: This is a lot of work, find a more efficient way to do this!
  for (iptr i = 0; i < count; ++i) {
    iptr tl, tr, bl, br;

    tl = tr = bl = br = 0;
    for (iptr j = 1; j < 4; ++j) {
      if ((quads[i].v[j].data.s <= quads[i].v[tl].data.s) &&
          (quads[i].v[j].data.t <= quads[i].v[tl].data.t))
      {
        tl = j;
      }

      if ((quads[i].v[j].data.s >= quads[i].v[tr].data.s) &&
          (quads[i].v[j].data.t <= quads[i].v[tr].data.t))
      {
        tr = j;
      }

      if ((quads[i].v[j].data.s <= quads[i].v[bl].data.s) &&
          (quads[i].v[j].data.t >= quads[i].v[bl].data.t))
      {
        bl = j;
      }

      if ((quads[i].v[j].data.s >= quads[i].v[br].data.s) &&
          (quads[i].v[j].data.t >= quads[i].v[br].data.t))
      {
        br = j;
      }
    }

    // If the quad doesn't represent a rectangle, don't setup corners
    if ((quads[i].v[tl].data.s != quads[i].v[bl].data.s) ||
        (quads[i].v[tl].data.t != quads[i].v[tr].data.t) ||
        (quads[i].v[tr].data.s != quads[i].v[br].data.s) ||
        (quads[i].v[bl].data.t != quads[i].v[br].data.t))
    {
      continue;
    }

    // Make sure all corner indices are unique
    // If left and right corner indices match, increment tl until vertex
    // with same Y coordinate is found, and do the same for bl
    if (tl == tr) {
      // If top and bottom corner indices also match, all corners have the same
      // texture coordinate.
      if (tl == bl) {
        out[i] = quads[i];
        continue;
      }

      // Increment tl until a vertex at the top is found
      do {
        tl = (tl + 1) & 3;
      } while (quads[i].v[tl].data.t != quads[i].v[tr].data.t);

      // Increment bl until a vertex at the bottom is found
      do {
        bl = (bl + 1) & 3;
      } while (quads[i].v[bl].data.t != quads[i].v[br].data.t);
    } else if (tl == bl) {
      // If the top and bottom corner indices match, increment tl until vertex
      // with same X coordinate is found, and do the same for tr
      ASSERT(tl != tr);

      // Increment tl until a vertex at the left is found
      do {
        tl = (tl + 1) & 3;
      } while (quads[i].v[tl].data.s != quads[i].v[bl].data.s);

      // Increment tr until a vertex at the right is found
      do {
        tr = (tr + 1) & 3;
      } while (quads[i].v[tr].data.s != quads[i].v[br].data.s);
    }

    out[i].v[0] = quads[i].v[tl];
    out[i].v[1] = quads[i].v[tr];
    out[i].v[2] = quads[i].v[bl];
    out[i].v[3] = quads[i].v[br];
  }

  [vertices didModifyRange:NSMakeRange(sizeof(rquad_t) * quadCount,
                                       sizeof(rquad_t) * count)];

  quadCount += count;
}

- (void)SetTexture:(const u32*)data left:(u32)left top:(u32)top
             right:(u32)right bottom:(u32)bottom
{
  [texture replaceRegion:MTLRegionMake2D(left, top, right - left, bottom - top)
             mipmapLevel:0
               withBytes:(unsigned char*)data
             bytesPerRow:4 * (right - left)];

  // If this region contains the clear color, reset the clear color pixel
  if (((u32)(CLRCOL_COORD_X - left) < (u32)(right - left)) &&
      ((u32)(CLRCOL_COORD_Y - top) < (u32)(bottom - top)))
  {
    SetClearColor(texture, clearColR, clearColG, clearColB);
  }
}

- (void)Render {
  @autoreleasepool {
    RenderView(self);
  }

  quadCount = 0;
}

- (void)UpdateDown {
  input.input.updateDown();
}

- (void)UpdateShift {
  // Only update if game processed input this frame
  if (!input.input.pressed) {
    if (input.pressShiftNextFrame) input.input.pressed |= INPUT_JUMPBIT;
    if (input.releaseShiftNextFrame) input.input.released |= INPUT_JUMPBIT;

    input.pressShiftNextFrame = input.releaseShiftNextFrame =
      input.lshiftPressed = input.lshiftReleased =
      input.rshiftPressed = input.rshiftReleased = false;
  }
}

- (input_t*)GetInput {
  return &input.input;
}

- (void)SetClearColor:(MTLClearColor)color {
  clearColR = color.red;
  clearColG = color.green;
  clearColB = color.blue;

  SetClearColor(texture, clearColR, clearColG, clearColB);
}

- (apple_viewport_t)GetViewport {
  return viewport;
}

////////////////////////////////////////////////////////////////////////////////
//      Keyboard event handling

- (BOOL)acceptsFirstResponder {
  return YES;
}

- (void)keyDown:(NSEvent*)evt {
  // 53 == escape
  if ([evt keyCode] == 53) [NSApp terminate:self];
  else input.input.pressed |= KeyCodeToField([evt keyCode]);
}

- (void)keyUp:(NSEvent*)evt {
  input.input.released |= KeyCodeToField([evt keyCode]);
}

- (void)flagsChanged:(NSEvent*)evt {
  ProcessFlagsChanged(&input, evt);
}

@end    //@implementation apple_view_t

static u16 *AllocIndexBuffer() {
  u16 *ret = (u16*)malloc(2 * 6 * MAX_QUADS);
  if (!ret) LOG_ERROR("Couldn't allocate index buffer!");

  for (iptr i = 0; i < MAX_QUADS; ++i) {
    u16 firstInd = i * 4;

    ret[i * 6 + 0] = firstInd + 0;
    ret[i * 6 + 1] = firstInd + 1;
    ret[i * 6 + 2] = firstInd + 2;
    ret[i * 6 + 3] = firstInd + 3;
    ret[i * 6 + 4] = firstInd + 2;
    ret[i * 6 + 5] = firstInd + 1;
  }

  return ret;
}

static id<MTLRenderPipelineState> CreatePipelineState(id<MTLDevice> device,
                                                      NSString *vertexFunc,
                                                      NSString *fragmentFunc)
{
  id<MTLLibrary> shaderLibrary = [[device newDefaultLibrary] autorelease];
  if (!shaderLibrary) LOG_ERROR("Couldn't get default shader library!");

  id<MTLFunction> vertexProgram =
    [shaderLibrary newFunctionWithName:vertexFunc];
  id<MTLFunction> fragmentProgram =
    [shaderLibrary newFunctionWithName:fragmentFunc];
  if (!vertexProgram || !fragmentProgram) {
    LOG_ERROR(FMT.s("Couldn't get ").s([vertexFunc UTF8String]).s(" and ")
              .s([fragmentFunc UTF8String]).s(" from shader library!").STR);
  }

  MTLRenderPipelineDescriptor *desc =
    [[[MTLRenderPipelineDescriptor alloc] init] autorelease];

  desc.label = @"Pipeline State";
  desc.vertexFunction = vertexProgram;
  desc.fragmentFunction = fragmentProgram;
  desc.colorAttachments[0].pixelFormat = PIXEL_FORMAT;
  desc.depthAttachmentPixelFormat = DEPTH_PIXEL_FORMAT;

  id<MTLRenderPipelineState> ret =
    [device newRenderPipelineStateWithDescriptor:desc
                                           error:nil];
  if (!ret) LOG_ERROR("Couldn't create pipeline state!");

  return ret;
}

static id<MTLTexture> CreateTexture(id<MTLDevice> device) {
  MTLTextureDescriptor *desc =
    [[[MTLTextureDescriptor alloc] init] autorelease];

  desc.pixelFormat = TEX_PIXEL_FORMAT;
  desc.width = 2048;
  desc.height = 2048;
  desc.textureType = MTLTextureType2D;
  desc.storageMode = MTLStorageModeShared;

  id<MTLTexture> ret = [device newTextureWithDescriptor:desc];
  if (!ret) LOG_ERROR("Couldn't create texture!");

  return ret;
}

static id<MTLDepthStencilState> CreateDepthState(id<MTLDevice> device) {
  MTLDepthStencilDescriptor *desc =
    [[[MTLDepthStencilDescriptor alloc] init] autorelease];

  desc.depthCompareFunction = MTLCompareFunctionLess;
  desc.depthWriteEnabled = TRUE;

  id<MTLDepthStencilState> ret =
    [device newDepthStencilStateWithDescriptor:desc];
  if (!ret) LOG_ERROR("Couldn't create depth state!");

  return ret;
}

static id<MTLTexture> CreateDepthBuf(id<MTLDevice> device,
                                     NSUInteger width, NSUInteger height)
{
  MTLTextureDescriptor *desc =
    [[[MTLTextureDescriptor alloc] init] autorelease];

  desc.pixelFormat = DEPTH_PIXEL_FORMAT;
  desc.width = width;
  desc.height = height;
  desc.textureType = MTLTextureType2D;
  desc.storageMode = MTLStorageModePrivate;
  desc.usage = MTLTextureUsageRenderTarget;

  id<MTLTexture> ret = [device newTextureWithDescriptor:desc];
  if (!ret) LOG_ERROR("Couldn't create depth buffer!");

  return ret;
}

static void RenderView(apple_view_t *view) {
  [view DrawQuads:&S_ClearQuad count:1];

  id<MTLCommandBuffer> cmdBuf = [view.cmdQueue commandBuffer];
  id<CAMetalDrawable> drawable = [view.metalLayer nextDrawable];

  if (!drawable) return;

  view.renderDescriptor.colorAttachments[0].texture = drawable.texture;

  id<MTLRenderCommandEncoder> renderEncoder =
    [cmdBuf renderCommandEncoderWithDescriptor:view.renderDescriptor];

  [renderEncoder setRenderPipelineState:view.pipelineState];
  [renderEncoder setDepthStencilState:view.depthState];

  [renderEncoder setVertexBuffer:view.vertices
                          offset:0
                         atIndex:0];
  [renderEncoder setFragmentTexture:view.texture
                            atIndex:0];

  apple_viewport_t viewport = [view GetViewport];

  MTLViewport metalViewport;
  metalViewport.originX = viewport.l;
  metalViewport.originY = viewport.t;
  metalViewport.width = viewport.w;
  metalViewport.height = viewport.h;
  metalViewport.znear = 0.0;
  metalViewport.zfar = 1.0;

  MTLScissorRect metalScissor;
  metalScissor.x = viewport.l;
  metalScissor.y = viewport.t;
  metalScissor.width = viewport.w;
  metalScissor.height = viewport.h;

  [renderEncoder setViewport:metalViewport];
  [renderEncoder setScissorRect:metalScissor];

  [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                            indexCount:(6 * view.quadCount)
                             indexType:MTLIndexTypeUInt16
                           indexBuffer:view.indices
                     indexBufferOffset:0];

  [renderEncoder endEncoding];

  [cmdBuf presentDrawable:drawable];
  [cmdBuf commit];
}

static void PressLeftShift(input_state_t *state) {
  if (state->lshiftReleased) state->pressShiftNextFrame = true;
  else if (!state->rshiftDown) state->input.pressed |= INPUT_JUMPBIT;

  state->lshiftDown = true;
  state->lshiftPressed = true;
}

static void PressRightShift(input_state_t *state) {
  if (state->rshiftReleased) state->pressShiftNextFrame = true;
  else if (!state->lshiftDown) state->input.pressed |= INPUT_JUMPBIT;

  state->rshiftDown = true;
  state->rshiftPressed = true;
}

static void ReleaseLeftShift(input_state_t *state) {
  if (state->lshiftPressed) state->releaseShiftNextFrame = true;
  else if (!state->rshiftDown) state->input.released |= INPUT_JUMPBIT;

  state->lshiftDown = false;
  state->lshiftReleased = true;
}

static void ReleaseRightShift(input_state_t *state) {
  if (state->rshiftPressed) state->releaseShiftNextFrame = true;
  else if (!state->lshiftDown) state->input.released |= INPUT_JUMPBIT;

  state->rshiftDown = false;
  state->rshiftReleased = true;
}

static ifast ModifierFlagsMask(NSEventModifierFlags flags) {
  if (flags & NSEventModifierFlagShift) return (flags >> 1) & 3;
  return 0;
}

static void ProcessFlagsChanged(input_state_t *state, NSEvent *evt) {
  ASSERT([evt type] == NSEventTypeFlagsChanged);

  ifast mask = ModifierFlagsMask([evt modifierFlags]);
  ifast diff = mask ^ state->lastModifierMask;

  if (diff & 1) {
    if (mask & 1) PressLeftShift(state);
    else ReleaseLeftShift(state);
  }

  if (diff & 2) {
    if (mask & 2) PressRightShift(state);
    else ReleaseRightShift(state);
  }

  state->lastModifierMask = mask;
}

static input_field_t KeyCodeToField(u16 keyCode) {
  switch (keyCode) {
  case 124: return INPUT_RIGHTBIT;
  case 126: return INPUT_UPBIT;
  case 123: return INPUT_LEFTBIT;
  case 125: return INPUT_DOWNBIT;
  case 6: return INPUT_SHOOTBIT;
  case 15: return INPUT_RESTARTBIT;
  case 120: return INPUT_NEWGAMEBIT;
  default: return 0;
  }
}

static void SetClearColor(id<MTLTexture> texture, f32 r, f32 g, f32 b) {
  u32 c = (0xff000000u | ((u32)(r * 255.f) << 16) | ((u32)(g * 255.f) << 8) |
           (u32)(b * 255.f));

  [texture replaceRegion:MTLRegionMake2D(CLRCOL_COORD_X, CLRCOL_COORD_Y, 1, 1)
             mipmapLevel:0
               withBytes:&c
             bytesPerRow:4];
}

static void CalcLetterBox(apple_viewport_t *output, NSUInteger width,
                          NSUInteger height, f32 targetRatio)
{
  f32 l, t, w, h;

  l = 0.f;
  t = 0.f;
  w = (f32)width / targetRatio;
  h = (f32)height;

  if (h < w) {
    l = (w - h) * 0.5f;
    w = h;
  } else if (h > w) {
    t = (h - w) * 0.5f;
    h = w;
  }

  l *= targetRatio;
  w *= targetRatio;

  output->l = (u32)l;
  output->t = (u32)t;
  output->w = (u32)w;
  output->h = (u32)h;
}
