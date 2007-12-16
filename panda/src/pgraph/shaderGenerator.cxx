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
//
//               Currently supports:
//               - textures
//               - materials
//               - lights
//
//               Not yet supported:
//               - vertex colors
//               - texgen
//               - texmatrix
//               - separate normal and gloss
//               - lots of other things
//
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderGenerator::
synthesize_shader(const RenderState *rs) {
  AttribSlots attribs;
  rs->store_into_slots(&attribs);
  
  int num_textures = 0;
  if (attribs._texture) {
    num_textures = attribs._texture->get_num_on_stages();
  }
  
  ostringstream text;
  CPT(RenderAttrib) rattrib = ShaderAttrib::make();

  text << "//Cg\n";

  text << "void vshader(\n";
  for (int i=0; i<num_textures; i++) {
    text << "\t float2 vtx_texcoord" << i << " : TEXCOORD" << i << ",\n";
    text << "\t out float2 l_texcoord" << i << " : TEXCOORD" << i << ",\n";
  }
  text << "\t float4 vtx_position : POSITION,\n";
  text << "\t out float4 l_position : POSITION,\n";
  text << "\t uniform float4x4 mat_modelproj\n";
  text << ") {\n";
  
  text << "\t l_position = mul(mat_modelproj, vtx_position);\n";

  for (int i=0; i<num_textures; i++) {
    text << "\t l_texcoord" << i << " = vtx_texcoord" << i << ";\n";
  }
  
  text << "}\n\n";

  text << "void fshader(\n";
  
  for (int i=0; i<num_textures; i++) {
    text << "\t in float2 l_texcoord" << i << " : TEXCOORD" << i << ",\n";
    text << "\t uniform sampler2D tex_" << i << ",\n";
  }
  text << "\t out float4 o_color : COLOR\n";
  text << ") {\n";
  
  text << "\t o_color = float4(1,1,1,1);\n";
  for (int i=0; i<num_textures; i++) {
    text << "\t o_color *= tex2D(tex_" << i << ", l_texcoord" << i << ");\n";
  }
  
  text << "}\n";

  // Insert the shader into the shader attrib.
  PT(Shader) shader = Shader::make(text.str());
  rattrib = DCAST(ShaderAttrib, rattrib)->set_shader(shader);
  return rattrib;
}

