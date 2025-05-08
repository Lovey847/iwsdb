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

// Pixel formats for framebuffers and textures
#define PIXEL_FORMAT MTLPixelFormatBGRA8Unorm
#define TEX_PIXEL_FORMAT MTLPixelFormatBGRA8Unorm
#define DEPTH_PIXEL_FORMAT MTLPixelFormatDepth32Float

#define PI 3.141596535897932384626f

// Maximum number of quads, quads drawn after this amount won't be rendered
#define MAX_QUADS 4096

// MacOS-specific modifier flags (I think anyway, haven't tested in iOS)
#define MODFLAG_LEFTSHIFT 2     // Whether the left shift key is held down
#define MODFLAG_RIGHTSHIFT 4    // Whether the right shift key is held down

#define MODFLAG_SHIFT_SHIFT 1
#define MODFLAG_SHIFT_MASK                                              \
  ((MODFLAG_LEFTSHIFT | MODFLAG_RIGHTSHIFT) >> MODFLAG_SHIFT_SHIFT)

// Mac key codes
#define MACKEY_ESCAPE 53
#define MACKEY_RIGHTARROW 124
#define MACKEY_UPARROW 126
#define MACKEY_LEFTARROW 123
#define MACKEY_DOWNARROW 125
#define MACKEY_Z 6
#define MACKEY_R 15
#define MACKEY_F2 120

// Coordinate clear color resides at in texture
#define CLRCOL_COORD_X 2047
#define CLRCOL_COORD_Y 2047

// Input state, including data for shift state
struct input_state_t {
  input_t input;

  bfast pressShiftNextFrame, releaseShiftNextFrame;
  bfast lshiftDown, lshiftPressed, lshiftReleased;
  bfast rshiftDown, rshiftPressed, rshiftReleased;

  ifast lastModifierMask;
};

// Allocate and initialize index buffer
static u16 *AllocIndexBuffer();
// Create pipeline state
static id<MTLRenderPipelineState> CreatePipelineState(id<MTLDevice> device,
                                                      NSString *vertexFunc,
                                                      NSString *fragmentFunc);
// Create texture
static id<MTLTexture> CreateTexture(id<MTLDevice> device);
// Create depth buffer state
static id<MTLDepthStencilState> CreateDepthState(id<MTLDevice> device);
// Create depth buffer texture
static id<MTLTexture> CreateDepthBuf(id<MTLDevice> device,
                                     NSUInteger width, NSUInteger height);
// Render view's contents
static void RenderView(apple_view_t *view);
// Process flags changed event (contains shift key events)
static void ProcessFlagsChanged(input_state_t *state, NSEvent *evt);
// Convert key code to input field
static input_field_t KeyCodeToField(u16 keyCode);
// Set clear color texel in texture
static void SetClearColor(id<MTLTexture> texture, f32 r, f32 g, f32 b);
// Calculate letter-boxed viewport
static void CalcLetterBox(apple_viewport_t *viewport, NSUInteger width,
                          NSUInteger height, f32 targetRatio);
// Process quad for use in vertex buffer
static void ProcessQuad(const rquad_t *input, rquad_t *output);

// Background color filling quad
static constexpr const rquad_t S_ClearQuad = {{
    {{{-1.f, 1.f, 0.9999999f, CLRCOL_COORD_X, CLRCOL_COORD_Y}}},
    {{{1.f, 1.f, 0.9999999f, CLRCOL_COORD_X + 1, CLRCOL_COORD_Y}}},
    {{{-1.f, -1.f, 0.9999999f, CLRCOL_COORD_X, CLRCOL_COORD_Y + 1}}},
    {{{1.f, -1.f, 0.9999999f, CLRCOL_COORD_X + 1, CLRCOL_COORD_Y + 1}}}
  }};

@implementation apple_view_t {
  id<MTLTexture> depthBuf;
  input_state_t input;

  // Current clear color
  f32 clearColR, clearColG, clearColB;

  // Current viewport
  apple_viewport_t viewport;
}

// Implement getters & setters for public members
@synthesize cmdQueue = cmdQueue;
@synthesize metalLayer = metalLayer;
@synthesize renderDescriptor = renderDescriptor;
@synthesize pipelineState = pipelineState;
@synthesize depthState = depthState;
@synthesize vertices = vertices;
@synthesize texture = texture;
@synthesize quadCount = quadCount;
@synthesize indices = indices;

// Initialize view
- (nonnull instancetype)initWithFrame:(NSRect)frame {
  self = [super initWithFrame:frame];
  if (self) {
    // Create hosted layer
    metalLayer = [CAMetalLayer layer];

    // Make this a layer-hosting view
    // According to apple documentation, you do this by setting the view's
    // layer, THEN setting that it wants a layer.
    [self setLayer:metalLayer];
    [self setWantsLayer:YES];

    // Create/get default metal device
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();

    [metalLayer setDevice:device];
    [metalLayer setPixelFormat:PIXEL_FORMAT];

    cmdQueue = [device newCommandQueue];

    depthBuf = CreateDepthBuf(device, (NSUInteger)frame.size.width,
                              (NSUInteger)frame.size.height);
    depthState = CreateDepthState(device);

    // Create render descriptor
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

    // Create vertex buffer
    vertices = [device newBufferWithLength:(MAX_QUADS * sizeof(rquad_t))
                                   options:MTLResourceStorageModeShared];
    if (!vertices) LOG_ERROR("Couldn't create vertex buffer!");

    // Create index buffer
    u16 *indexBufferContents = AllocIndexBuffer();
    indices =
      [device newBufferWithBytes:indexBufferContents
                          length:(MAX_QUADS * 6 * sizeof(u16))
                         options:(MTLResourceStorageModeShared |
                                  MTLResourceHazardTrackingModeUntracked)];
    if (!indices) LOG_ERROR("Couldn't create index buffer!");
    free(indexBufferContents);

    texture = CreateTexture(device);

    // Initialize quad rendering state
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
  // Get default metal device
  id<MTLDevice> device = MTLCreateSystemDefaultDevice();

  // Create new metal layer which has current size of view
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
  // Truncate quad count if limit has been reached
  if (quadCount + count >= MAX_QUADS) count = MAX_QUADS - quadCount;

  // If there are no quads to draw, exit
  if (count <= 0) return;

  // Process quads into vertex buffer
  rquad_t *out = (rquad_t*)[vertices contents] + quadCount;

  for (iptr i = 0; i < count; ++i) ProcessQuad(quads + i, out + i);

  // Tell metal this range of the vertex buffer has been modified
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
  // Only update shift state if game processed input this frame
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

// This view receives keyboard events
- (BOOL)acceptsFirstResponder {
  return YES;
}

// A key other than shift was pressed
- (void)keyDown:(NSEvent*)evt {
  if ([evt isARepeat]) return;

  // Close the application if the user pressed escape
  if ([evt keyCode] == MACKEY_ESCAPE) [NSApp terminate:self];
  else input.input.pressed |= KeyCodeToField([evt keyCode]);
}

- (void)keyUp:(NSEvent*)evt {
  if ([evt isARepeat]) return;

  input.input.released |= KeyCodeToField([evt keyCode]);
}

// A shift key may have been pressed
- (void)flagsChanged:(NSEvent*)evt {
  ProcessFlagsChanged(&input, evt);
}

@end    //@implementation apple_view_t

static u16 *AllocIndexBuffer() {
  // 2 bytes per index, 6 indices per quad
  u16 *ret = (u16*)malloc(2 * 6 * MAX_QUADS);
  if (!ret) LOG_ERROR("Couldn't allocate index buffer!");

  // Initialize index buffer with indices for a list of MAX_QUADS quads
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
  // Load default.metallib
  id<MTLLibrary> shaderLibrary = [[device newDefaultLibrary] autorelease];
  if (!shaderLibrary) LOG_ERROR("Couldn't get default shader library!");

  // Get vertex and fragment shader from library
  id<MTLFunction> vertexProgram =
    [shaderLibrary newFunctionWithName:vertexFunc];
  id<MTLFunction> fragmentProgram =
    [shaderLibrary newFunctionWithName:fragmentFunc];
  if (!vertexProgram || !fragmentProgram) {
    LOG_ERROR(FMT.s("Couldn't get ").s([vertexFunc UTF8String]).s(" and ")
              .s([fragmentFunc UTF8String]).s(" from shader library!").STR);
  }

  // Create pipeline descriptor with vertex and fragment shader
  MTLRenderPipelineDescriptor *desc =
    [[[MTLRenderPipelineDescriptor alloc] init] autorelease];

  desc.label = @"Pipeline State";
  desc.vertexFunction = vertexProgram;
  desc.fragmentFunction = fragmentProgram;
  desc.colorAttachments[0].pixelFormat = PIXEL_FORMAT;
  desc.depthAttachmentPixelFormat = DEPTH_PIXEL_FORMAT;

  // Create pipeline state from pipeline descriptor
  id<MTLRenderPipelineState> ret =
    [device newRenderPipelineStateWithDescriptor:desc
                                           error:nil];
  if (!ret) LOG_ERROR("Couldn't create pipeline state!");

  return ret;
}

static id<MTLTexture> CreateTexture(id<MTLDevice> device) {
  // Create 2048x2048 texture
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
  // Create depth buffer state whose depth test passes when the depth of the
  // fragment is less than the depth in the depth buffer
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
  // Create depth buffer texture
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
  // Render background
  [view DrawQuads:&S_ClearQuad count:1];

  // Get comand buffer from command queue
  id<MTLCommandBuffer> cmdBuf = [view.cmdQueue commandBuffer];

  // Get drawable from metal layer
  id<CAMetalDrawable> drawable = [view.metalLayer nextDrawable];

  if (!drawable) return;

  // Initialize render descriptor color attachment texture
  view.renderDescriptor.colorAttachments[0].texture = drawable.texture;

  // Create render command encoder
  id<MTLRenderCommandEncoder> renderEncoder =
    [cmdBuf renderCommandEncoderWithDescriptor:view.renderDescriptor];

  // Encode commands to set the pipeline state and depth buffer state
  [renderEncoder setRenderPipelineState:view.pipelineState];
  [renderEncoder setDepthStencilState:view.depthState];

  // Encode commands to set the vertex buffer
  [renderEncoder setVertexBuffer:view.vertices
                          offset:0
                         atIndex:0];

  // Encode commands to set the texture
  [renderEncoder setFragmentTexture:view.texture
                            atIndex:0];

  // Encode commands to set the viewport and scissor
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

  // Encode commands to draw the quads in the vertex buffer
  [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                            indexCount:(6 * view.quadCount)
                             indexType:MTLIndexTypeUInt16
                           indexBuffer:view.indices
                     indexBufferOffset:0];

  // Done encoding commands into cmdBuf
  [renderEncoder endEncoding];

  // Present drawable after render commands are done executing
  [cmdBuf presentDrawable:drawable];

  // Submit render commands to the GPU
  [cmdBuf commit];
}

static void PressLeftShift(input_state_t *state) {
  // If left shift was released this frame, avoid a single-shift cancel by
  // queueing to press jump on the next frame.
  // Otherwise, if right shift isn't also down, press jump on this frame.
  if (state->lshiftReleased) state->pressShiftNextFrame = true;
  else if (!state->rshiftDown) state->input.pressed |= INPUT_JUMPBIT;

  // Set left shift state
  state->lshiftDown = true;
  state->lshiftPressed = true;
}

static void PressRightShift(input_state_t *state) {
  // If right shift was released this frame, avoid a single-shift cancel by
  // queueing to press jump on the next frame.
  // Otherwise, if left shift isn't also down, press jump on this frame.
  if (state->rshiftReleased) state->pressShiftNextFrame = true;
  else if (!state->lshiftDown) state->input.pressed |= INPUT_JUMPBIT;

  // Set right shift state
  state->rshiftDown = true;
  state->rshiftPressed = true;
}

static void ReleaseLeftShift(input_state_t *state) {
  // If left shift was pressed this frame, avoid a single-shift cancel by
  // queueing to release jump on the next frame.
  // Otherwise, if right shift isn't also down, release jump on this frame.
  if (state->lshiftPressed) state->releaseShiftNextFrame = true;
  else if (!state->rshiftDown) state->input.released |= INPUT_JUMPBIT;

  // Set left shift state
  state->lshiftDown = false;
  state->lshiftReleased = true;
}

static void ReleaseRightShift(input_state_t *state) {
  // If right shift was pressed this frame, avoid a single-shift cancel by
  // queueing to release jump on the next frame.
  // Otherwise, if left shift isn't also down, release jump on this frame.
  if (state->rshiftPressed) state->releaseShiftNextFrame = true;
  else if (!state->lshiftDown) state->input.released |= INPUT_JUMPBIT;

  // Set right shift state
  state->rshiftDown = false;
  state->rshiftReleased = true;
}

// Returns 0 if neither shift key is down, 1 if left shift is down,
// 2 if right shift is down, or 3 if both shifts are down
static ifast ModifierFlagsMask(NSEventModifierFlags flags) {
  if (flags & NSEventModifierFlagShift)
    return (flags >> MODFLAG_SHIFT_SHIFT) & MODFLAG_SHIFT_MASK;

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
  case MACKEY_RIGHTARROW: return INPUT_RIGHTBIT;
  case MACKEY_UPARROW: return INPUT_UPBIT;
  case MACKEY_LEFTARROW: return INPUT_LEFTBIT;
  case MACKEY_DOWNARROW: return INPUT_DOWNBIT;
  case MACKEY_Z: return INPUT_SHOOTBIT;
  case MACKEY_R: return INPUT_RESTARTBIT;
  case MACKEY_F2: return INPUT_NEWGAMEBIT;

  default: return 0;
  }
}

static void SetClearColor(id<MTLTexture> texture, f32 r, f32 g, f32 b) {
  // Convert floats to u8s and pack into a BGRA color
  u32 c =
    LittleEndian32(0xff000000u | ((u32)(r * 255.f) << 16) |
                   ((u32)(g * 255.f) << 8) | (u32)(b * 255.f));

  // Set clear color texel in texture
  [texture replaceRegion:MTLRegionMake2D(CLRCOL_COORD_X, CLRCOL_COORD_Y, 1, 1)
             mipmapLevel:0
               withBytes:&c
             bytesPerRow:4];
}

static void CalcLetterBox(apple_viewport_t *output, NSUInteger width,
                          NSUInteger height, f32 targetRatio)
{
  f32 l, t, w, h;

  // Shrink width by aspect ratio, making the desired viewport a square
  l = 0.f;
  t = 0.f;
  w = (f32)width / targetRatio;
  h = (f32)height;

  // If the height is less than the width, shrink the width to make a square,
  // and center the viewport horizontally.
  // Otherwise, if the height is greater than the width, shrink the height to
  // make a square, and center the viewport vertically.
  if (h < w) {
    l = (w - h) * 0.5f;
    w = h;
  } else if (h > w) {
    t = (h - w) * 0.5f;
    h = w;
  }

  // Stretch width back out, giving the desired aspect ratio
  l *= targetRatio;
  w *= targetRatio;

  output->l = (u32)l;
  output->t = (u32)t;
  output->w = (u32)w;
  output->h = (u32)h;
}

static void ProcessQuad(const rquad_t *input, rquad_t *output) {
  const vertex_t *v = input->v;
  vertex_t *ov = output->v;

  // Arrange the vertices where v[0] is the top left, v[1] is the top right,
  // v[2] is the bottom left, and v[3] is the bottom right of the image
  // in the texture page.
  // For an explanation of why this is done, see CoordOffsets in
  // src/plat/apple_shaders.metal.
  iptr tl = 0, tr = 1, bl = 2, br = 3;

  // 2 possible orders (row major or column major),
  // 4 possible directions (going down right, going down left, going up right,
  // going up left)

  // If the order is column major, swizzle corners appropriately
  if (v[0].data.s == v[1].data.s) {
    tr = 2;
    bl = 1;
  }

  // If the rectangle is pointing left, swap tl&tr and bl&br
  if (v[tl].data.s > v[tr].data.s) {
    iptr tmp = tl;
    tl = tr;
    tr = tmp;
    tmp = bl;
    bl = br;
    br = tmp;
  }

  // If the rectangle is pointing up, swap tl&bl and tr&br
  if (v[tl].data.t > v[bl].data.t) {
    iptr tmp = tl;
    tl = bl;
    bl = tmp;
    tmp = tr;
    tr = br;
    br = tmp;
  }

  ov[0] = v[tl];
  ov[1] = v[tr];
  ov[2] = v[bl];
  ov[3] = v[br];
}
