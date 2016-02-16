/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lineParticleRenderer.I
 * @author darren
 * @date 2000-10-06
 */

/**

 */
INLINE void LineParticleRenderer::
set_head_color(const LColor& c) {
  _head_color = c;
}

/**

 */
INLINE void LineParticleRenderer::
set_tail_color(const LColor& c) {
  _tail_color = c;
}

/**

 */
INLINE const LColor& LineParticleRenderer::
get_head_color() const {
  return _head_color;
}

/**

 */
INLINE const LColor& LineParticleRenderer::
get_tail_color() const {
  return _tail_color;
}

/**
 * accessor
 */
INLINE void LineParticleRenderer::
set_line_scale_factor(PN_stdfloat sf) {
  _line_scale_factor = sf;
}

/**
 * accessor
 */
INLINE PN_stdfloat LineParticleRenderer::
get_line_scale_factor() const {
  return _line_scale_factor;
}
