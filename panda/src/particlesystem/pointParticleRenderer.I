/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointParticleRenderer.I
 * @author charles
 * @date 2000-06-20
 */

/**

 */
INLINE void PointParticleRenderer::
set_point_size(PN_stdfloat point_size) {
  _point_size = point_size;
  _thick = RenderModeAttrib::make(RenderModeAttrib::M_unchanged, _point_size);
}

/**

 */
INLINE void PointParticleRenderer::
set_start_color(const LColor& sc) {
  _start_color = sc;
}

/**

 */
INLINE void PointParticleRenderer::
set_end_color(const LColor& ec) {
  _end_color = ec;
}

/**

 */
INLINE void PointParticleRenderer::
set_blend_type(PointParticleRenderer::PointParticleBlendType bt) {
  _blend_type = bt;
}

/**

 */
INLINE void PointParticleRenderer::
set_blend_method(BaseParticleRenderer::ParticleRendererBlendMethod bm) {
  _blend_method = bm;
}

/**

 */
INLINE PN_stdfloat PointParticleRenderer::
get_point_size() const {
  return _point_size;
}

/**

 */
INLINE const LColor& PointParticleRenderer::
get_start_color() const {
  return _start_color;
}

/**

 */
INLINE const LColor& PointParticleRenderer::
get_end_color() const {
  return _end_color;
}

/**

 */
INLINE PointParticleRenderer::PointParticleBlendType PointParticleRenderer::
get_blend_type() const {
  return _blend_type;
}

/**

 */
INLINE BaseParticleRenderer::ParticleRendererBlendMethod PointParticleRenderer::
get_blend_method() const {
  return _blend_method;
}
