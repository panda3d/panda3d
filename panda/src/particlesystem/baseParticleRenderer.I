/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file baseParticleRenderer.I
 * @author charles
 * @date 2000-06-20
 */

/**
 * Query the geomnode pointer
 */
INLINE GeomNode *BaseParticleRenderer::
get_render_node() const {
  return _render_node;
}

/**
 * Query the geomnode pointer
 */
INLINE NodePath BaseParticleRenderer::
get_render_node_path() const {
  return _render_node_path;
}

/**

 */
INLINE void BaseParticleRenderer::
set_alpha_mode(BaseParticleRenderer::ParticleRendererAlphaMode am) {
  update_alpha_mode(am);
  init_geoms();
}

/**

 */
INLINE BaseParticleRenderer::ParticleRendererAlphaMode BaseParticleRenderer::
get_alpha_mode() const {
  return _alpha_mode;
}

/**
 * sets alpha for "user" alpha mode
 */
INLINE void BaseParticleRenderer::
set_user_alpha(PN_stdfloat ua) {
  _user_alpha = ua;
}

/**
 * gets alpha for "user" alpha mode
 */
INLINE PN_stdfloat BaseParticleRenderer::
get_user_alpha() const {
  return _user_alpha;
}

/**
 * sets the ColorBlendAttrib on the _render_node
 */
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
}

/**
 * Returns the "ignore scale" flag.  See set_ignore_scale().
 */
INLINE bool BaseParticleRenderer::
get_ignore_scale() const {
  return _ignore_scale;
}

/**
 * gets current alpha for a particle
 */
INLINE PN_stdfloat BaseParticleRenderer::
get_cur_alpha(BaseParticle* bp) {
  switch(_alpha_mode) {
  case PR_ALPHA_OUT:
    return 1.0f - bp->get_parameterized_age();

  case PR_ALPHA_IN:
    return bp->get_parameterized_age();

  case PR_ALPHA_IN_OUT:
    return 2.0 * std::min(bp->get_parameterized_age(),
                     1.0f - bp->get_parameterized_age());

  case PR_ALPHA_USER:
    return _user_alpha;

  default:
    return 1.0; // should not get here
  }
}
