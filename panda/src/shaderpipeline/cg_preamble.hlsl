/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cg_preamble.hlsl
 * @author rdb
 * @date 2020-06-10
 */

/**
 * This file gets loaded as preamble to any Cg shader to be able to compile it
 * through the HLSL front-end of the glslang compiler.  It defines functions
 * and types that were supported in Cg but are not available in HLSL.
 */

typedef sampler2D sampler2DShadow;

float4 shadow2D(sampler2D samp, float3 s) {
  return tex2D(samp, s.xy).r > s.z;
}

float4 shadow2DProj(sampler2D samp, float4 s) {
  return tex2Dproj(samp, s).r > s.z / s.w;
}

float4 f4tex1D(sampler1D samp, float s) {
  return tex1D(samp, s);
}

float4 f4tex1D(sampler1D samp, float2 s) {
  return tex1D(samp, s.x).r > s.y;
}

float4 f4tex1D(sampler1D samp, float s, float dx, float dy) {
  return tex1D(samp, s, dx, dy);
}

float4 f4tex1D(sampler1D samp, float2 s, float dx, float dy) {
  return tex1D(samp, s.x, dx, dy).r > s.y;
}

float4 f4tex1Dproj(sampler1D samp, float2 s) {
  return tex1Dproj(samp, s.xxxy);
}

float4 f4tex1Dproj(sampler1D samp, float3 s) {
  return tex1Dproj(samp, s.xxxz).r > s.y / s.z;
}

float4 f4tex1Dproj(sampler1D samp, float2 s, float dx, float dy) {
  return tex1D(samp, s.x / s.y, dx, dy);
}

float4 f4tex1Dproj(sampler1D samp, float3 s, float dx, float dy) {
  return tex1D(samp, s.x / s.z).r > s.y / s.z;
}

float4 f4tex2D(sampler2D samp, float2 s) {
  return tex2D(samp, s.xy);
}

float4 f4tex2D(sampler2D samp, float3 s) {
  return tex2D(samp, s.xy).r > s.z;
}

float4 f4tex2D(sampler2D samp, float2 s, float2 dx, float2 dy) {
  return tex2D(samp, s.xy, dx, dy);
}

float4 f4tex2D(sampler2D samp, float3 s, float2 dx, float2 dy) {
  return tex2D(samp, s.xy, dx, dy).r > s.z;
}

float4 f4tex2Dproj(sampler2D samp, float3 s) {
  return tex2Dproj(samp, s.xyzz);
}

float4 f4tex2Dproj(sampler2D samp, float4 s) {
  return tex2Dproj(samp, s).r > s.z / s.w;
}

float4 f4tex2Dproj(sampler2D samp, float3 s, float2 dx, float2 dy) {
  return tex2D(samp, s.xyyz, dx, dy);
}

float4 f4tex2Dproj(sampler2D samp, float4 s, float2 dx, float2 dy) {
  return tex2D(samp, s.xy / s.w, dx, dy).r > s.z / s.w;
}

/*
float4 f4texCUBE(samplerCUBE samp, float3 s) {
  return texCUBE(samp, s);
}

float4 f4texCUBE(samplerCUBE samp, float4 s) {
  return texCUBE(samp, s.xyz).r > s.w;
}

float4 f4texCUBE(samplerCUBE samp, float3 s, float3 dx, float3 dy) {
  return texCUBE(samp, s, dx, dy);
}

float4 f4texCUBE(samplerCUBE samp, float4 s, float3 dx, float3 dy) {
  return texCUBE(samp, s.xyz, dx, dy).r > s.w;
}
*/

#define f1tex1D(x, y) (f4tex1D((x), (y)).r)
#define f3tex1D(x, y) (f4tex1D((x), (y)).rgb)
#define tex1D f4tex1D

#define f1tex1Dproj(x, y) (f4tex1Dproj((x), (y)).r)
#define f3tex1Dproj(x, y) (f4tex1Dproj((x), (y)).rgb)
#define tex1Dproj f4tex1Dproj

#define f1tex2D(x, y) (f4tex2D((x), (y)).r)
#define f3tex2D(x, y) (f4tex2D((x), (y)).rgb)
#define tex2D f4tex2D

#define f1tex2Dproj(x, y) (f4tex2Dproj((x), (y)).r)
#define f3tex2Dproj(x, y) (f4tex2Dproj((x), (y)).rgb)
#define tex2Dproj f4tex2Dproj

/*
#define f1texCUBE(x, y) (f4texCUBE((x), (y)).r)
#define f3texCUBE(x, y) (f4texCUBE((x), (y)).rgb)
#define texCUBE f4texCUBE
*/
