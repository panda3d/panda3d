/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fisheyeMaker.I
 * @author drose
 * @date 2005-10-03
 */

/**
 *
 */
INLINE FisheyeMaker::
FisheyeMaker(const std::string &name) : Namable(name) {
  reset();
}

/**
 *
 */
INLINE FisheyeMaker::
~FisheyeMaker() {
}

/**
 * Specifies the approximate number of vertices to be used to generate the
 * rose.  This is the approximate number of vertices that will be located
 * within the rose's unit circle, not counting the inscribing square (if any).
 * The actual number of vertices used may be +/- 25% of this value.
 */
INLINE void FisheyeMaker::
set_num_vertices(int num_vertices) {
  _num_vertices = num_vertices;
}

/**
 * Sets the flag that indicates whether the rose should be inscribed within a
 * square.  When this is true, an additional square is generated to inscribed
 * the circular rose, with the indicated "radius" (the sides of the square
 * will be 2 * square_radius).  The texture coordinates of the square will
 * uniformly map to the back pole of the cube map.
 *
 * This is mainly useful to provide a good uniform background color for a
 * sphere map so that it does not have a sharp circular edge that might
 * produce artifacts due to numerical imprecision when mapping.
 */
INLINE void FisheyeMaker::
set_square_inscribed(bool square_inscribed, PN_stdfloat square_radius) {
  _square_inscribed = square_inscribed;
  _square_radius = square_radius;
}

/**
 * Sets the flag indicating whether the texture image should be mirrored
 * (true) or normal (false).  When this is true, the 3-D texture coordinates
 * will be reversed so that the image is appropriate for a reflection.  This
 * is the best choice for generating a sphere map from a cube map.  The
 * default is false.
 */
INLINE void FisheyeMaker::
set_reflection(bool reflection) {
  _reflect = (reflection) ? -1.0 : 1.0;
}
