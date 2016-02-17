/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file antialiasAttrib.I
 * @author drose
 * @date 2005-01-26
 */

/**
 * Use AntialiasAttrib::make() to construct a new AntialiasAttrib object.
 */
INLINE AntialiasAttrib::
AntialiasAttrib(unsigned short mode) :
  _mode(mode)
{
}

/**
 * Returns the specified antialias mode.
 */
INLINE unsigned short AntialiasAttrib::
get_mode() const {
  return _mode;
}

/**
 * Returns the specified antialias mode, with the quality bits masked out.
 * This therefore indicates only the requested type of antialiasing: M_none,
 * M_auto, or some specific combination.
 */
INLINE unsigned short AntialiasAttrib::
get_mode_type() const {
  return _mode & M_type_mask;
}

/**
 * Returns the specified antialias mode, with the type bits masked out.  This
 * therefore indicates only the requested quality settings: one of M_faster,
 * M_better, M_dont_care, or zero (unspecified).
 */
INLINE unsigned short AntialiasAttrib::
get_mode_quality() const {
  return _mode & ~M_type_mask;
}
