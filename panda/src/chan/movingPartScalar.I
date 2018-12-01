/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movingPartScalar.I
 * @author drose
 * @date 1999-02-23
 */

/**
 * Normally, you'd use make_copy() or copy_subgraph() to make a copy of this.
 */
INLINE MovingPartScalar::
MovingPartScalar(const MovingPartScalar &copy) :
  MovingPart<ACScalarSwitchType>(copy)
{
}

/**
 *
 */
INLINE MovingPartScalar::
MovingPartScalar(PartGroup *parent, const std::string &name,
                 const PN_stdfloat &default_value)
  : MovingPart<ACScalarSwitchType>(parent, name, default_value) {
}

/**
 *
 */
INLINE MovingPartScalar::
MovingPartScalar(){
}
