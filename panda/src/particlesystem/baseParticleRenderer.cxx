// Filename: baseParticleRenderer.cxx
// Created by:  charles (20Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"

#include "baseParticleRenderer.h"
#include "transparencyAttrib.h"

////////////////////////////////////////////////////////////////////
//    Function : BaseParticleRenderer
//      Access : Public
// Description : Default Constructor
////////////////////////////////////////////////////////////////////
BaseParticleRenderer::
BaseParticleRenderer(ParticleRendererAlphaMode alpha_mode) :
  _alpha_mode(PR_NOT_INITIALIZED_YET) {
  _render_node = new GeomNode("BaseParticleRenderer render node");

  _user_alpha = 1.0f;

  update_alpha_mode(alpha_mode);
}

////////////////////////////////////////////////////////////////////
//    Function : BaseParticleRenderer
//      Access : Public
// Description : Copy Constructor
////////////////////////////////////////////////////////////////////
BaseParticleRenderer::
BaseParticleRenderer(const BaseParticleRenderer& copy) :
  _alpha_mode(PR_ALPHA_NONE) {
  _render_node = new GeomNode("BaseParticleRenderer render node");

  _user_alpha = copy._user_alpha;

  update_alpha_mode(copy._alpha_mode);
}

////////////////////////////////////////////////////////////////////
//    Function : ~BaseParticleRenderer
//      Access : Public
// Description : Destructor
////////////////////////////////////////////////////////////////////
BaseParticleRenderer::
~BaseParticleRenderer() {
}

////////////////////////////////////////////////////////////////////
//    Function : enable_alpha
//      Access : Private
// Description : Builds an intermediate node and transition that
//               enables alpha channeling.
////////////////////////////////////////////////////////////////////
void BaseParticleRenderer::
enable_alpha() {
  _render_state = RenderState::make(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
}

////////////////////////////////////////////////////////////////////
//    Function : disable_alpha
//      Access : Private
// Description : kills the intermediate alpha node/arc
////////////////////////////////////////////////////////////////////
void BaseParticleRenderer::
disable_alpha() {
  _render_state = RenderState::make(TransparencyAttrib::make(TransparencyAttrib::M_none));
}

////////////////////////////////////////////////////////////////////
//    Function : update_alpha_state
//      Access : Private
// Description : handles the base class part of alpha updating.
////////////////////////////////////////////////////////////////////
void BaseParticleRenderer::
update_alpha_mode(ParticleRendererAlphaMode am) {
  if (_alpha_mode == am)
    return;

  if ((am == PR_ALPHA_NONE) && (_alpha_mode != PR_ALPHA_NONE))
    disable_alpha();
  else if ((am != PR_ALPHA_NONE) && (_alpha_mode == PR_ALPHA_NONE))
    enable_alpha();

  _alpha_mode = am;
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void BaseParticleRenderer::
output(ostream &out, unsigned int indent) const {
  out.width(indent); out<<""; out<<"BaseParticleRenderer:\n";
  //ReferenceCount::output(out, indent+2);
}
