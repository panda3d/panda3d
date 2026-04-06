/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file heightfieldTesselator.I
 * @author jyelon
 * @date 2006-07-17
 */

/**
 *
 */
INLINE HeightfieldTesselator::
HeightfieldTesselator(const std::string &name) : Namable(name) {
  _poly_count = 10000;
  _visibility_radius = 32768;
  _focal_x = 0;
  _focal_y = 0;
  _horizontal_scale = 1.0;
  _vertical_scale = 255.0;
  _max_triangles = 512;
  _radii_calculated = false;
}

/**
 *
 */
INLINE HeightfieldTesselator::
~HeightfieldTesselator() {
}

/**
 * Returns a reference to the heightfield (a PNMImage) contained inside the
 * HeightfieldTesselator.  You can use the reference to alter the heightfield.
 */
INLINE PNMImage &HeightfieldTesselator::
heightfield() {
  return _heightfield;
}

/**
 * Loads the specified greyscale image file into the heightfield.
 */
INLINE bool HeightfieldTesselator::
set_heightfield(const Filename &filename, PNMFileType *ftype) {
  _radii_calculated = false;
  return _heightfield.read(filename, ftype);
}

/**
 * Sets the polygon-count target.  The tesselator usually manages to come
 * within about 20% of the target, plus or minus.
 */
INLINE void HeightfieldTesselator::
set_poly_count(int n) {
  _radii_calculated = false;
  _poly_count = n;
}

/**
 * Sets the visibility radius.  Polygons that are completely outside the
 * radius (relative to the focal point) are cropped away.  The cropping is
 * imperfect (all approximations are conservative), so this should be used in
 * conjunction with a far clipping plane, fog, or some other visibility
 * limiting mechanism.  The units are in pixels.
 */
INLINE void HeightfieldTesselator::
set_visibility_radius(int radius) {
  _radii_calculated = false;
  if (radius < 1) radius = 1;
  if (radius > 32768) radius = 32768;
  _visibility_radius = radius;
}

/**
 * Sets the focal point.  The tesselator generates high-resolution terrain
 * around the focal point, and progressively lower and lower resolution
 * terrain as you get farther away.  The units are in pixels.
 */
INLINE void HeightfieldTesselator::
set_focal_point(int x, int y) {
  _focal_x = x;
  _focal_y = y;
}

/**
 * Sets the horizontal scale.  The default scale is 1.0, meaning that each
 * pixel in the heightfield is 1x1 panda units wide.
 */
INLINE void HeightfieldTesselator::
set_horizontal_scale(double h) {
  _horizontal_scale = h;
}

/**
 * Sets the vertical scale.  The default scale is 255.0, meaning that each as
 * the gray value ranges from (0-1), the elevation ranges from (0-255) feet.
 */
INLINE void HeightfieldTesselator::
set_vertical_scale(double v) {
  _vertical_scale = v;
}

/**
 * Sets the max triangles per geom.
 */
INLINE void HeightfieldTesselator::
set_max_triangles(int n) {
  _max_triangles = n;
}

/**
 * Returns true if the given square should be subdivided.
 */
INLINE bool HeightfieldTesselator::
subdivide(int scale, int x, int y) {
  if (scale == 0) {
    return false;
  }
  // int size = 1<<scale; int hsize = size >> 1; int xcenter = x+hsize; int
  // ycenter = y+hsize;
  int deltax = x - _focal_x;
  int deltay = y - _focal_y;
  if (deltax < 0) deltax = -deltax;
  if (deltay < 0) deltay = -deltay;
  int dist = (deltax > deltay) ? deltax : deltay;
  if (dist < _radii[scale-1]) {
    return true;
  }
  return false;
}
