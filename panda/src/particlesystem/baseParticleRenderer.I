// Filename: baseParticleRenderer.I
// Created by:  charles (20Jun00)
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

////////////////////////////////////////////////////////////////////
//    Function : get_render_node
//       Class : Public
// Description : Query the geomnode pointer
////////////////////////////////////////////////////////////////////
INLINE GeomNode *BaseParticleRenderer::
get_render_node() const {
  return _render_node;
}

////////////////////////////////////////////////////////////////////
//    Function : set_alpha_mode
//      Access : public
////////////////////////////////////////////////////////////////////
INLINE void BaseParticleRenderer::
set_alpha_mode(BaseParticleRenderer::ParticleRendererAlphaMode am) {
  update_alpha_mode(am);
  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : get_alpha_mode
//      Access : public
////////////////////////////////////////////////////////////////////
INLINE BaseParticleRenderer::ParticleRendererAlphaMode BaseParticleRenderer::
get_alpha_mode() const {
  return _alpha_mode;
}

////////////////////////////////////////////////////////////////////
//    Function : set_user_alpha
//      Access : public
// Description : sets alpha for "user" alpha mode
////////////////////////////////////////////////////////////////////
INLINE void BaseParticleRenderer::
set_user_alpha(float ua) {
  _user_alpha = ua;
}

////////////////////////////////////////////////////////////////////
//    Function : get_user_alpha
//      Access : public
// Description : gets alpha for "user" alpha mode
////////////////////////////////////////////////////////////////////
INLINE float BaseParticleRenderer::
get_user_alpha() const {
  return _user_alpha;
}

////////////////////////////////////////////////////////////////////
//    Function : set_color_blend_mode
//      Access : public
// Description : sets the ColorBlendAttrib on the _render_node
////////////////////////////////////////////////////////////////////
INLINE void BaseParticleRenderer::
set_color_blend_mode(ColorBlendAttrib::Mode bm, ColorBlendAttrib::Operand oa, ColorBlendAttrib::Operand ob) {
  CPT(RenderAttrib) ra;
  if(bm == ColorBlendAttrib::M_add || bm == ColorBlendAttrib::M_subtract || bm == ColorBlendAttrib::M_inv_subtract) {
    ra = ColorBlendAttrib::make(bm,oa,ob);
  } else {
    ra = ColorBlendAttrib::make(bm);
  }

  _render_node->set_attrib(ra);
  return;
};

////////////////////////////////////////////////////////////////////
//    Function : get_cur_alpha
//      Access : public
// Description : gets current alpha for a particle
////////////////////////////////////////////////////////////////////
INLINE float BaseParticleRenderer::
get_cur_alpha(BaseParticle* bp) {
  switch(_alpha_mode) {
  case PR_ALPHA_OUT:
    return 1.0f - bp->get_parameterized_age();

  case PR_ALPHA_IN:
    return bp->get_parameterized_age();

  case PR_ALPHA_IN_OUT:
    return 2.0 * min(bp->get_parameterized_age(),
                     1.0f - bp->get_parameterized_age());

  case PR_ALPHA_USER:
    return _user_alpha;

  default:
    return 1.0; // should not get here
  }
}
