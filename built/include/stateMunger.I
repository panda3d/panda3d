/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stateMunger.I
 * @author drose
 * @date 2005-05-04
 */

/**
 *
 */
INLINE StateMunger::
StateMunger(GraphicsStateGuardianBase *gsg) :
  GeomMunger(gsg),
  _should_munge_state(false)
{
}

/**
 * Returns true if this munger has something interesting to do to the state.
 */
INLINE bool StateMunger::
should_munge_state() const {
  return _should_munge_state;
}
