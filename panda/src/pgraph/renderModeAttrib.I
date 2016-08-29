/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderModeAttrib.I
 * @author drose
 * @date 2002-03-14
 */

/**
 * Use RenderModeAttrib::make() to construct a new RenderModeAttrib object.
 */
INLINE RenderModeAttrib::
RenderModeAttrib(RenderModeAttrib::Mode mode, PN_stdfloat thickness,
                 bool perspective, const LColor &wireframe_color) :
  _mode(mode),
  _thickness(thickness),
  _perspective(perspective),
  _wireframe_color(wireframe_color)
{
}

/**
 * Returns the render mode.
 */
INLINE RenderModeAttrib::Mode RenderModeAttrib::
get_mode() const {
  return _mode;
}

/**
 * Returns the line width or point thickness.  This is only relevant when
 * rendering points or lines, such as when the mode is M_wireframe or M_point
 * (or when rendering actual points or lines primitives in M_polygon mode).
 */
INLINE PN_stdfloat RenderModeAttrib::
get_thickness() const {
  return _thickness;
}

/**
 * Returns the perspective flag.  When this is true, the point thickness
 * represented by get_thickness() is actually a width in 3-d units, and the
 * points should scale according to perspective.  When it is false, the
 * default, the point thickness is actually a width in pixels, and points are
 * a uniform size regardless of distance from the camera.
 */
INLINE bool RenderModeAttrib::
get_perspective() const {
  return _perspective;
}

/**
 * Returns the color that is used in M_filled_wireframe mode to distinguish
 * the wireframe from the rest of the geometry.
 */
INLINE const LColor &RenderModeAttrib::
get_wireframe_color() const {
  return _wireframe_color;
}

/**
 * Returns the union of the Geom::GeomRendering bits that will be required
 * once this RenderModeAttrib is applied to a geom which includes the
 * indicated geom_rendering bits.
 */
INLINE int RenderModeAttrib::
get_geom_rendering(int geom_rendering) const {
  if (_mode == M_point) {
    geom_rendering |= Geom::GR_point | Geom::GR_render_mode_point;

  } else if (_mode == M_wireframe) {
    geom_rendering |= Geom::GR_render_mode_wireframe;
  }

  if ((geom_rendering & Geom::GR_point) != 0) {
    if (_perspective) {
      geom_rendering |= (Geom::GR_point_perspective | Geom::GR_point_uniform_size);
    } else if (_thickness != 1.0f) {
      geom_rendering |= Geom::GR_point_uniform_size;
    }
  }

  return geom_rendering;
}
