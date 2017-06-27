//
//  Shaders.metal
//  ss
//
//  Created by Samuco on 23/6/16.
//  Copyright (c) 2016 Samuco. All rights reserved.
//

#include <metal_stdlib>
#include <simd/simd.h>
#include "SharedStructures.h"

using namespace metal;

// Variables in constant address space
constant float3 light_position = float3(0.0, 1.0, -1.0);
constant float4 ambient_color  = float4(0.18, 0.24, 0.8, 1.0);
constant float4 diffuse_color  = float4(0.4, 0.4, 1.0, 1.0);

typedef struct
{
    float3 position [[attribute(0)]];
    float2 texcoord [[attribute(1)]];
    float2 litcoord [[attribute(2)]];
    float3 normal   [[attribute(3)]];
    float3 binormal [[attribute(4)]];
    float3 tangent  [[attribute(5)]];
} vertex_t;

typedef struct {
    float4 position [[position]];
    float2 texcoord;
    float2 litcoord;
    half4  color;
} ColorInOut;

// Vertex shader function
vertex ColorInOut lighting_vertex(vertex_t vertex_array [[stage_in]],
                                  constant senv_uniform& uniforms [[ buffer(1) ]])
{
    ColorInOut out;
    
    float4 in_position = float4(vertex_array.position, 1.0);
    out.position = uniforms.shared.modelview_projection_matrix * in_position;
    out.texcoord = vertex_array.texcoord;
    out.litcoord = vertex_array.litcoord;
    
    float4 eye_normal = normalize(uniforms.shared.normal_matrix * float4(vertex_array.normal, 0.0));
    float n_dot_l = dot(eye_normal.rgb, normalize(light_position));
    n_dot_l = fmax(0.0, n_dot_l);
    
    out.color = half4(ambient_color + diffuse_color * n_dot_l);
    return out;
}

// Fragment shader function
fragment float4 lighting_fragment(ColorInOut in [[stage_in]],
                                  texture2d<float> baseMap [[texture(0)]],
                                  texture2d<float> primaryMap [[texture(1)]],
                                  texture2d<float> secondaryMap [[texture(2)]],
                                  sampler texSample [[sampler(0)]],
                                  constant senv_uniform& uniforms [[ buffer(1) ]])
{
    float2 texCoord = float2(in.texcoord.r * uniforms.options.scaleU, in.texcoord.g * uniforms.options.scaleV);
    float4 baseColor = baseMap.sample(texSample, texCoord);
    float3 primaryColor = primaryMap.sample(texSample, texCoord * uniforms.options.primaryScale).rgb;
    float3 secondaryColor = secondaryMap.sample(texSample, uniforms.options.secondaryScale * texCoord).rgb;
    float3 white = float3(1.0, 1.0, 1.0);
    float3 primaryMix = mix(white, 2 * primaryColor, uniforms.options.mixPrimary);
    float3 secondaryMix = mix(white, 2 * secondaryColor, uniforms.options.mixSecondary);
    float3 detailColor  = mix(secondaryMix, primaryMix, baseColor.a);
    return float4(baseColor.rgb * detailColor, 1.0);
}
