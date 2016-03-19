/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file auxBitplaneAttrib.I
 * @author drose
 * @date 2002-03-04
 */

/**
 * Use AuxBitplaneAttrib::make() to construct a new AuxBitplaneAttrib object.
 */
INLINE AuxBitplaneAttrib::
AuxBitplaneAttrib(int outputs) :
  _outputs(outputs)
{
}

/**
 * Returns the AuxBitplaneAttrib output bits.
 */
INLINE int AuxBitplaneAttrib::
get_outputs() const {
  return _outputs;
}
