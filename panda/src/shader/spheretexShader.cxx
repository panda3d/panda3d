// Filename: spheretexShader.cxx
// Created by:  mike (09Jan97)
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
#include "spheretexShader.h"
#include "config_shader.h"

#include "lightTransition.h"
#include "textureTransition.h"
#include "textureTransition.h"
#include "texMatrixTransition.h"
#include "depthTestTransition.h"
#include "texGenTransition.h"
#include "textureApplyTransition.h"
#include "colorBlendTransition.h"
#include "get_rel_pos.h"
#include "dftraverser.h"
#include "graphicsStateGuardian.h"
#include "colorBlendTransition.h"
#include "attribTraverser.h"
#include "directRenderTraverser.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle SpheretexShader::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: SpheretexShader::constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
SpheretexShader::SpheretexShader(Texture* texture) : Shader()
{
  set_texture(texture);
  _blend_mode = ColorBlendProperty::M_multiply_add;
}

////////////////////////////////////////////////////////////////////
//     Function: SpheretexShader::config
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void SpheretexShader::config(void)
{
  Configurable::config();

  nassertv(_texture != (Texture *)NULL);
  _texture->set_minfilter(Texture::FT_linear);
  _texture->set_magfilter(Texture::FT_linear);
  _texture->set_wrapu(Texture::WM_clamp);
  _texture->set_wrapv(Texture::WM_clamp);
}

////////////////////////////////////////////////////////////////////
//     Function: SpheretexShader::apply
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void SpheretexShader::
apply(Node *node, const AllAttributesWrapper &init_state,
      const AllTransitionsWrapper &net_trans, GraphicsStateGuardian *gsg) {

  Shader::apply(node, init_state, net_trans, gsg);

  // Copy the transition wrapper so we can modify it freely.
  AllTransitionsWrapper trans(net_trans);

  // Create the texture generation transition
  TexGenTransition *tg = new TexGenTransition;
  tg->set_sphere_map();
  trans.set_transition(tg);

  // Create the texture transition
  nassertv(_texture != (Texture *)NULL);
  TextureTransition *t = new TextureTransition;
  t->set_on(_texture);
  t->set_priority(_priority);
  trans.set_transition(t);

  if (_viz != (Shader::Visualize*)0L)
    _viz->DisplayTexture(_texture, this);

  // Clear the texture matrix transition
  trans.set_transition(new TexMatrixTransition(LMatrix4f::ident_mat()));

  // Turn lighting off - we still want to issue normals, though
  trans.set_transition(new LightTransition(LightTransition::all_off()));
  gsg->force_normals();

  // Do some extra work if we're doing the 2-pass version (e.g. the
  // object we are shading is textured)
  if (_multipass_on) {
    // Set a color blend mode that assumes this is a second pass over
    // textured geometry
    ColorBlendTransition *cb =
      new ColorBlendTransition(_blend_mode);
    trans.set_transition(cb);

    TextureApplyTransition *ta =
      new TextureApplyTransition(TextureApplyProperty::M_decal);
    trans.set_transition(ta);

    // Set the depth test to M_equal (? Or should this be M_none?)
    DepthTestTransition *dta =
      new DepthTestTransition(DepthTestProperty::M_equal);
    trans.set_transition(dta);
  }

  // Render the node with the new transitions.
  DirectRenderTraverser drt(gsg, RenderRelation::get_class_type());
  gsg->render_subgraph(&drt, node, init_state, trans);

  gsg->undo_force_normals();
}



