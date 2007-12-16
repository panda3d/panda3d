// Filename: shaderGenerator.cxx
// Created by: jyelon (15Dec07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "renderState.h"
#include "shaderAttrib.h"
#include "shaderGenerator.h"

////////////////////////////////////////////////////////////////////
//     Function: ShaderGenerator::synthesize_shader
//       Access: Public, Static
//  Description: This is the routine that implements the next-gen
//               fixed function pipeline by synthesizing a shader.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderGenerator::
synthesize_shader(const RenderState *rs) {

  // Stub code: just return this shader.
  
  PT(Shader) shader = Shader::make(
    "//Cg \n"
    "// \n"
    "//Cg profile arbvp1 arbfp1 \n"
    " \n"
    "void vshader(float4 vtx_position   : POSITION, \n"
    "             float3 vtx_normal     : NORMAL, \n"
    "             float4 vtx_color      : COLOR, \n"
    "             out float4 l_position : POSITION, \n"
    "             out float4 l_brite    : TEXCOORD0, \n"
    "             out float4 l_color    : COLOR, \n"
    "             uniform float4 mspos_light, \n"
    "             uniform float4x4 mat_modelproj) \n"
    "{ \n"
    "  l_position = mul(mat_modelproj, vtx_position); \n"
    "  float3 N = normalize(vtx_normal); \n"
    "  float3 lightVector = normalize(mspos_light - vtx_position); \n"
    "  l_brite = max(dot(N,lightVector), 0); \n"
    "  l_color = vtx_color; \n"
    "} \n"
    " \n"
    " \n"
    "void fshader(float4 l_brite     : TEXCOORD0,  \n"
    "             float4 l_color     : COLOR, \n"
    "             out float4 o_color : COLOR) \n"
    "{ \n"
    "  if (l_brite.x<0.5) l_brite=0.8; \n"
    "  else l_brite=1.2; \n"
    "  o_color=l_brite * l_color; \n"
    "} \n"
    );
  CPT(RenderAttrib) rattr = rs->get_shader();
  if (rattr == 0) rattr = ShaderAttrib::make();
  return DCAST(ShaderAttrib, rattr)->set_shader(shader);
}

