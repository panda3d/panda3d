/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterVertexSlider.I
 * @author drose
 * @date 2005-03-28
 */

/**
 * Returns the CharacterSlider object for which this object returns the slider
 * value.
 */
INLINE const CharacterSlider *CharacterVertexSlider::
get_char_slider() const {
  return _char_slider;
}
