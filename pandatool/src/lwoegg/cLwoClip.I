/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cLwoClip.I
 * @author drose
 * @date 2001-04-27
 */

/**
 * Returns the index number of this clip.  Each clip in a Lightwave object
 * file should have a unique index number.
 */
int CLwoClip::
get_index() const {
  return _clip->_index;
}

/**
 * Returns true if this clip represents a still image, as opposed to an
 * animated image.  If this is true, _filename will contain the image
 * filename.
 */
bool CLwoClip::
is_still_image() const {
  return _still_image;
}
