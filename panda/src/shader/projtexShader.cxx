// Filename: projtexShader.cxx
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
#include "projtexShader.h"
#include "config_shader.h"

#include "lightTransition.h"
#include "textureTransition.h"
#include "get_rel_pos.h"
#include "dftraverser.h"
#include "colorBlendTransition.h"
#include "depthTestTransition.h"
#include "graphicsStateGuardian.h"
#include "texGenTransition.h"
#include "texMatrixTransition.h"
#include "perspectiveProjection.h"
#include "attribTraverser.h"
#include "textureApplyTransition.h"
#include "directRenderTraverser.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle ProjtexShader::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ProjtexShader::constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
ProjtexShader::ProjtexShader(Texture* texture,
                             ColorBlendProperty::Mode mode)
  : FrustumShader(), _blend(mode)
{
  set_texture(texture);
}

////////////////////////////////////////////////////////////////////
//     Function: ProjtexShader::config
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void ProjtexShader::config(void)
{
  Configurable::config();

  nassertv(_texture != (Texture *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: ProjtexShader::apply
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void ProjtexShader::
apply(Node *node, const AllAttributesWrapper &init_state,
      const AllTransitionsWrapper &net_trans, GraphicsStateGuardian *gsg) {

  Shader::apply(node, init_state, net_trans, gsg);

  // Make sure the shader has been configured properly
  if (get_num_frusta() == 0) {
    shader_cat.error()
      << "ProjtexShader::apply() - frusta list is empty" << endl;
    return;
  } else if (get_num_frusta() > 1) {
    shader_cat.warning()
      << "ProjtexShader::apply() - frusta list has more than one "
      << "- ignoring all but first one for now..." << endl;
  }

  // Copy the transition wrapper so we can modify it freely.
  AllTransitionsWrapper trans(net_trans);

  // Create the texture projector transition
  // NOTE: If the node being projected upon has its own textures,
  // we need to render it first with its own texturing before
  // this projected texture pass and then blend the results
  LMatrix4f model_mat;
  const Projection* projection = _frusta[0]->get_projection();
  get_rel_mat(node, _frusta[0], model_mat);

  // Create the texture generation transition
  TexGenTransition *tg = new TexGenTransition;
  tg->set_texture_projector();
  trans.set_transition(tg);
  // Create the texture transition
  nassertv(_texture != (Texture *)NULL);
  TextureTransition *t = new TextureTransition;
  t->set_on(_texture);
  t->set_priority(_priority);
  trans.set_transition(t);

  if (_viz != (Shader::Visualize*)0L)
    _viz->DisplayTexture(_texture, this);
  // Create the texture matrix transition
  // Define texture matrix so that object space coords used as
  // tex coords will be projected onto the frustum's viewing plane
  // An additional scale and translate is required to go from x,y
  // NDC coords in [-1,1] to tex coords [0,1]
  LMatrix4f tex_mat, proj_mat;
  const PerspectiveProjection *pp = DCAST(PerspectiveProjection, projection);
  proj_mat = pp->get_projection_mat();
  tex_mat = model_mat * proj_mat *
    LMatrix4f::scale_mat(LVector3f(0.5,0.5,1)) *
    LMatrix4f::translate_mat(LVector3f(0.5,0.5,0));

  TexMatrixTransition *tm = new TexMatrixTransition;
  tm->set_matrix(tex_mat);
  trans.set_transition(tm);

  // Turn lighting off
  trans.set_transition(new LightTransition(LightTransition::all_off()));
  // Do some extra work if we're doing the 2-pass version (e.g. the
  // object we are shading is textured)
  if (_multipass_on) {
    //Probably should move the brains for determing blending out to
    //shader transition

    // Set a color blend mode that assumes this is a second pass over
    // textured geometry
    ColorBlendTransition *cb = new ColorBlendTransition(_blend);
    trans.set_transition(cb);

    TextureApplyTransition *ta =
      new TextureApplyTransition(TextureApplyProperty::M_decal);
    trans.set_transition(ta);

    // Set the depth test to M_equal (? Or should this be M_none?)
    DepthTestTransition *dta =
      new DepthTestTransition(DepthTestProperty::M_equal);
    trans.set_transition(dta);
  }

  DirectRenderTraverser drt(gsg, RenderRelation::get_class_type());
  gsg->render_subgraph(&drt, node, init_state, trans);
}



