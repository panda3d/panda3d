/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file audioSound.I
 * @author jyelon
 * @date 2007-08-01
 */

/**
 * Returns true if this was created as a positional sound.
 */
INLINE bool AudioSound::
is_positional() const {
  return _positional;
}
