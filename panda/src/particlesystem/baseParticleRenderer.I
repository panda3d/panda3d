// Filename: baseParticleRenderer.I
// Created by:  charles (20Jun00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//    Function : get_render_node
//       Class : Public
// Description : Query the geomnode pointer
////////////////////////////////////////////////////////////////////

INLINE GeomNode *BaseParticleRenderer::
get_render_node(void) const {
  return _render_node;
}

////////////////////////////////////////////////////////////////////
//    Function : get_alpha_decay
//      Access : Public
////////////////////////////////////////////////////////////////////

INLINE ParticleRendererAlphaDecay BaseParticleRenderer::
get_alpha_decay(void) const {
  return _alpha_decay;
}

////////////////////////////////////////////////////////////////////
//    Function : disable_alpha
//      Access : Private
// Description : kills the intermediate alpha node/arc
////////////////////////////////////////////////////////////////////

INLINE void BaseParticleRenderer::
disable_alpha(void) {
  _alpha_node.clear();

  if (_alpha_arc.is_null() == false) {
    remove_arc(_alpha_arc);
    _alpha_arc.clear();
  }

  _interface_node = _render_node;
}

////////////////////////////////////////////////////////////////////
//    Function : set_alpha_decay
//      Access : public
////////////////////////////////////////////////////////////////////

INLINE void BaseParticleRenderer::
set_alpha_decay(ParticleRendererAlphaDecay ad) {
  update_alpha_state(ad);
  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : color_lerp
//      Access : Protected
// Description : Linear interpolate 2 color vectors
////////////////////////////////////////////////////////////////////

INLINE Colorf BaseParticleRenderer::
color_lerp(float t, const Colorf& c1, const Colorf& c2) {
  return (c1 + (c2 - c1) * t);
}

////////////////////////////////////////////////////////////////////
//    Function : color_clerp
//      Access : Protected
// Description : Cubic interpolation between 2 color vectors.
////////////////////////////////////////////////////////////////////

INLINE Colorf BaseParticleRenderer::
color_clerp(float t, const Colorf& c1, const Colorf& c2) {
  float cubic_t = t * t * ((2 * t) - 3);
  return color_lerp(cubic_t, c1, c2);
}

////////////////////////////////////////////////////////////////////
//    Function : cubic_smooth
//      Access : Protected
// Description : smooths out a linear parameterized t in [0,1]
////////////////////////////////////////////////////////////////////

INLINE float BaseParticleRenderer::
cubic_smooth(float t) {
  return t * t * (3 - (2 * t));
}

////////////////////////////////////////////////////////////////////
//    Function : lerp
//      Access : Protected
////////////////////////////////////////////////////////////////////

INLINE float BaseParticleRenderer::
lerp(float t, float x0, float x1) {
  return x0 + (t * (x1 - x0));
}
