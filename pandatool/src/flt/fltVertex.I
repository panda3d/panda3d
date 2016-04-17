/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltVertex.I
 * @author drose
 * @date 2000-08-30
 */

/**
 * Returns true if the vertex has a primary color indicated, false otherwise.
 */
INLINE bool FltVertex::
has_color() const {
  // Even if the no_color bit is not set, if the color_index is -1, the vertex
  // doesn't have a color (unless we've got packed color).
  return ((_flags & F_no_color) == 0 &&
          (_color_index != -1 || ((_flags & F_packed_color) != 0)));
}

/**
 * Sets the color of the vertex, using the packed color convention.  The alpha
 * component is ignored.
 */
INLINE void FltVertex::
set_color(const LColor &color) {
  set_rgb(LRGBColor(color[0], color[1], color[2]));
}
