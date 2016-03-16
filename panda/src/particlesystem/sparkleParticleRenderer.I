/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sparkleParticleRenderer.I
 * @author charles
 * @date 2000-06-27
 */

/**

 */
INLINE void SparkleParticleRenderer::
set_center_color(const LColor& c) {
  _center_color = c;
}

/**

 */
INLINE void SparkleParticleRenderer::
set_edge_color(const LColor& c) {
  _edge_color = c;
}

/**

 */
INLINE void SparkleParticleRenderer::
set_life_scale(SparkleParticleRenderer::SparkleParticleLifeScale ls) {
  _life_scale = ls;
}

/**

 */
INLINE void SparkleParticleRenderer::
set_birth_radius(PN_stdfloat radius) {
  _birth_radius = radius;
}

/**

 */
INLINE void SparkleParticleRenderer::
set_death_radius(PN_stdfloat radius) {
  _death_radius = radius;
}

/**

 */
INLINE const LColor& SparkleParticleRenderer::
get_center_color() const {
  return _center_color;
}

/**

 */
INLINE const LColor& SparkleParticleRenderer::
get_edge_color() const {
  return _edge_color;
}

/**

 */
INLINE SparkleParticleRenderer::SparkleParticleLifeScale SparkleParticleRenderer::
get_life_scale() const {
  return _life_scale;
}

/**

 */
INLINE PN_stdfloat SparkleParticleRenderer::
get_birth_radius() const {
  return _birth_radius;
}

/**

 */
INLINE PN_stdfloat SparkleParticleRenderer::
get_death_radius() const {
  return _death_radius;
}

/**

 */
INLINE PN_stdfloat SparkleParticleRenderer::
get_radius(BaseParticle *bp) {
  if (_life_scale == SP_NO_SCALE)
    return _birth_radius;
  else {
    PN_stdfloat s_x = CUBIC_T(bp->get_parameterized_age());
    return LERP(s_x, _birth_radius, _death_radius);
  }
}
