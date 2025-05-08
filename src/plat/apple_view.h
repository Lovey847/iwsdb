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

#ifndef _PLAT_APPLE_VIEW_H
#define _PLAT_APPLE_VIEW_H

#include "loveylib/types.h"
#include "vertex.h"
#include "game.h"

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>

struct apple_viewport_t {
  u32 l, t;     // Left and top
  u32 w, h;     // Width and height
};

// Layer-hosting window view, handles keyboard events and rendering.
@interface apple_view_t : NSView {
  id<MTLCommandQueue> cmdQueue;
  // Layer hosted by view
  CAMetalLayer *metalLayer;
  // Render pass descriptor, contains framebuffer properties
  MTLRenderPassDescriptor *renderDescriptor;
  // Pipeline state, contains shaders and framebuffer pixel formats
  id<MTLRenderPipelineState> pipelineState;
  // Depth buffer state, contains depth test calculation method
  id<MTLDepthStencilState> depthState;
  // Vertex buffer
  id<MTLBuffer> vertices;
  // Internal 2048x2048 texture
  id<MTLTexture> texture;
  // Current number of quads in vertex buffer
  iptr quadCount;
  // Index buffer, constant buffer containing "0,1,2,3,2,1,4,5,6,7,6,5,..."
  id<MTLBuffer> indices;
}

// Make all properties public
@property (nonatomic,weak,readwrite) id<MTLCommandQueue> cmdQueue;
@property (nonatomic,weak,readwrite) CAMetalLayer *metalLayer;
@property (nonatomic,weak,readwrite) MTLRenderPassDescriptor *renderDescriptor;
@property (nonatomic,weak,readwrite) id<MTLRenderPipelineState> pipelineState;
@property (nonatomic,weak,readwrite) id<MTLDepthStencilState> depthState;
@property (nonatomic,weak,readwrite) id<MTLBuffer> vertices;
@property (nonatomic,weak,readwrite) id<MTLTexture> texture;
@property (nonatomic,assign,readwrite) iptr quadCount;
@property (nonatomic,weak,readwrite) id<MTLBuffer> indices;

// Updates the size the game is rendered at to the size of the view
- (void)UpdateRenderSize;
// Renders quads
- (void)DrawQuads:(const rquad_t*)quads count:(iptr)count;
// Set portion of internal 2048x2048 texture
- (void)SetTexture:(const u32*)data left:(u32)left top:(u32)top
             right:(u32)right bottom:(u32)bottom;
// Render into view
- (void)Render;
// Update which keys are held down
- (void)UpdateDown;
// Update shift state
- (void)UpdateShift;
// Get input data from window
- (input_t*)GetInput;
// Set renderer clear color
- (void)SetClearColor:(MTLClearColor)color;
// Get current viewport
- (apple_viewport_t)GetViewport;

@end    //@interface apple_view_t

#endif  //ifndef _PLAT_APPLE_VIEW_H
