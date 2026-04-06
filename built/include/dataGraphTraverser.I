/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dataGraphTraverser.I
 * @author drose
 * @date 2002-03-11
 */

/**
 * Returns the currently-executing thread object, as passed to the
 * DataGraphTraverser constructor.
 */
INLINE Thread *DataGraphTraverser::
get_current_thread() const {
  return _current_thread;
}

/**
 *
 */
INLINE DataGraphTraverser::CollectedData::
CollectedData() :
  _num_parents(0)
{
}
