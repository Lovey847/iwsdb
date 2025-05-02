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

vertex vertex_output_t VertexShader(uint vertexID [[vertex_id]],
                                    constant vertex_t *vertices [[buffer(0)]])
{
  vertex_output_t out;

  out.clipSpacePos.xyz = vertices[vertexID].pos;
  out.clipSpacePos.z = (out.clipSpacePos.z + 1.0) * 0.5;
  out.clipSpacePos.w = 1.0;

  out.coord = float2(vertices[vertexID].coord);

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
