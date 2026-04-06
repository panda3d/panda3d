/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggSaver.I
 * @author drose
 * @date 2012-12-19
 */

/**
 * Returns the EggData populated within this class.
 */
INLINE EggData *EggSaver::
get_egg_data() const {
  return _data;
}
