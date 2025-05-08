/************************************************************
 *
 * Copyright (c) 2025 Lian Ferrand
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

#include <metal_stdlib>

using namespace metal;

struct vertex_t {
  packed_float3 pos;
  ushort2 coord;
};

struct vertex_output_t {
  float4 clipSpacePos [[position]];
  float2 coord;
};

// Metal sometimes grabs texels from outside the texture coordinates when
// drawing scaled images with nearest neighbor filtering.  The correct solution
// would be to pad the edges of the images in the pages, however that's too big
// a change for a port so instead the corner tex coords will be slightly nudged
// into the image.
static constant float2 CoordOffset[4] = {
  float2(0.01, 0.01),
  float2(-0.01, 0.01),
  float2(0.01, -0.01),
  float2(-0.01, -0.01)
};

vertex vertex_output_t VertexShader(uint vertexID [[vertex_id]],
                                    constant vertex_t *vertices [[buffer(0)]])
{
  vertex_output_t out;

  out.clipSpacePos.xyz = vertices[vertexID].pos;
  out.clipSpacePos.z = (out.clipSpacePos.z + 1.0) * 0.5;
  out.clipSpacePos.w = 1.0;

  out.coord = float2(vertices[vertexID].coord);
  out.coord += CoordOffset[vertexID & 3];

  return out;
}

fragment float4
FragmentShader(vertex_output_t in [[stage_in]],
               texture2d<half> colorTex [[texture(0)]])
{
  constexpr sampler texSampler(mag_filter::nearest, min_filter::nearest,
                               coord::pixel);
  const half4 texSample = colorTex.sample(texSampler, in.coord);
  if (texSample.a < 0.5) discard_fragment();
  return float4(texSample);
}
