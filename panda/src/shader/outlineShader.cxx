// Filename: outlineShader.cxx
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
#include "outlineShader.h"
#include "lightTransition.h"
#include "dftraverser.h"
#include "depthTestTransition.h"
#include "graphicsStateGuardian.h"
#include "attribTraverser.h"
#include "renderModeTransition.h"
#include "cullFaceTransition.h"
#include "colorTransition.h"
#include "linesmoothTransition.h"
#include "directRenderTraverser.h"
#include "renderRelation.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle OutlineShader::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: OutlineShader::constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
OutlineShader::OutlineShader(void) : Shader() {
  set_color(Colorf(0, 0, 0, 1));
}

////////////////////////////////////////////////////////////////////
//     Function: OutlineShader::constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
OutlineShader::OutlineShader(const Colorf &color) : Shader() {
  set_color(color);
}

////////////////////////////////////////////////////////////////////
//     Function: OutlineShader::config
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void OutlineShader::config(void) {
  Configurable::config();
}

////////////////////////////////////////////////////////////////////
//     Function: OutlineShader::apply
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void OutlineShader::
apply(Node *node, const AllAttributesWrapper &init_state,
      const AllTransitionsWrapper &net_trans, GraphicsStateGuardian *gsg) {
  Shader::apply(node, init_state, net_trans, gsg);
  DirectRenderTraverser drt(gsg, RenderRelation::get_class_type());

  // If the node is un-textured, we need to render it once first (since
  // the shader transition won't have drawn it unless it is textured
  if (!is_textured(node, init_state)) {
    gsg->render_subgraph(&drt, node, init_state, net_trans);
  }

  // Copy the transition wrapper so we can modify it freely.
  AllTransitionsWrapper trans(net_trans);

  // Turn lighting off
  trans.set_transition(new LightTransition(LightTransition::all_off()));

  // Enable line drawing
  RenderModeTransition *rm =
    new RenderModeTransition(RenderModeProperty::M_wireframe, 2.0);
  trans.set_transition(rm);

  // Enable line smooth
  //  LinesmoothTransition *lsm = new LinesmoothTransition;
  //  trans.set_transition(lsm);

  // Draw shared edges
  DepthTestTransition *dta =
    new DepthTestTransition(DepthTestProperty::M_less_equal);
  trans.set_transition(dta);

  // Draw back facing edges only
  CullFaceTransition *cf =
    new CullFaceTransition(CullFaceProperty::M_cull_counter_clockwise);
  trans.set_transition(cf);

  // Set the outline color
  ColorTransition *c = new ColorTransition(_color);
  trans.set_transition(c);

  gsg->render_subgraph(&drt, node, init_state, trans);
}
