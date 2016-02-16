/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dynamicTextPage.I
 * @author drose
 * @date 2002-02-09
 */

/**
 * Returns the size of the page (texture), in pixels.
 */
INLINE const LVecBase2i &DynamicTextPage::
get_size() const {
  return _size;
}

/**
 * Returns the x size of the page (texture), in pixels.
 */
INLINE int DynamicTextPage::
get_x_size() const {
  return _size.get_x();
}

/**
 * Returns the y size of the page (texture), in pixels.
 */
INLINE int DynamicTextPage::
get_y_size() const {
  return _size.get_y();
}

/**
 * Returns true if the page has no glyphs, false otherwise.
 */
INLINE bool DynamicTextPage::
is_empty() const {
  return _glyphs.empty();
}
