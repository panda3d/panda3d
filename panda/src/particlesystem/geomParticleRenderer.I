/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomParticleRenderer.I
 * @author charles
 * @date 2000-07-05
 */

/**

 */

// we're forcing a pool resize to remove every node in the vector.  this is
// because nodes are reference-counted, and we have to explicitly remove them
// from the node they were previously parented to.

INLINE void GeomParticleRenderer::
set_geom_node(PandaNode *node) {
  nassertv(node != nullptr);
  _geom_node = node;
  resize_pool(_pool_size);
}

/**

 */
INLINE void GeomParticleRenderer::
set_x_scale_flag(bool animate_x_ratio) {
  _animate_x_ratio = animate_x_ratio;
  init_geoms();
}

/**

 */
INLINE void GeomParticleRenderer::
set_y_scale_flag(bool animate_y_ratio) {
  _animate_y_ratio = animate_y_ratio;
  init_geoms();
}

/**

 */
INLINE void GeomParticleRenderer::
set_z_scale_flag(bool animate_z_ratio) {
  _animate_z_ratio = animate_z_ratio;
  init_geoms();
}

/**

 */
INLINE void GeomParticleRenderer::
set_initial_x_scale(PN_stdfloat initial_x_scale) {
  _initial_x_scale = initial_x_scale;
  init_geoms();
}

/**

 */
INLINE void GeomParticleRenderer::
set_final_x_scale(PN_stdfloat final_x_scale) {
  _final_x_scale = final_x_scale;
  init_geoms();
}

/**

 */
INLINE void GeomParticleRenderer::
set_initial_y_scale(PN_stdfloat initial_y_scale) {
  _initial_y_scale = initial_y_scale;
  init_geoms();
}

/**

 */
INLINE void GeomParticleRenderer::
set_final_y_scale(PN_stdfloat final_y_scale) {
  _final_y_scale = final_y_scale;
  init_geoms();
}

/**

 */
INLINE void GeomParticleRenderer::
set_initial_z_scale(PN_stdfloat initial_z_scale) {
  _initial_z_scale = initial_z_scale;
  init_geoms();
}

/**

 */
INLINE void GeomParticleRenderer::
set_final_z_scale(PN_stdfloat final_z_scale) {
  _final_z_scale = final_z_scale;
  init_geoms();
}

/**

 */
INLINE PandaNode *GeomParticleRenderer::
get_geom_node() {
  return _geom_node;
}

/**

 */
INLINE ColorInterpolationManager* GeomParticleRenderer::
get_color_interpolation_manager() const {
  return _color_interpolation_manager;
}

/**

 */
INLINE bool GeomParticleRenderer::
get_x_scale_flag() const {
  return _animate_x_ratio;
}

/**

 */
INLINE bool GeomParticleRenderer::
get_y_scale_flag() const {
  return _animate_y_ratio;
}

/**

 */
INLINE bool GeomParticleRenderer::
get_z_scale_flag() const {
  return _animate_z_ratio;
}

/**

 */
INLINE PN_stdfloat GeomParticleRenderer::
get_initial_x_scale() const {
  return _initial_x_scale;
}

/**

 */
INLINE PN_stdfloat GeomParticleRenderer::
get_final_x_scale() const {
  return _final_x_scale;
}

/**

 */
INLINE PN_stdfloat GeomParticleRenderer::
get_initial_y_scale() const {
  return _initial_y_scale;
}

/**

 */
INLINE PN_stdfloat GeomParticleRenderer::
get_final_y_scale() const {
  return _final_y_scale;
}

/**

 */
INLINE PN_stdfloat GeomParticleRenderer::
get_initial_z_scale() const {
  return _initial_z_scale;
}

/**

 */
INLINE PN_stdfloat GeomParticleRenderer::
get_final_z_scale() const {
  return _final_z_scale;
}
