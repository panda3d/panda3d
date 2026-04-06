/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file genericThread.I
 * @author drose
 * @date 2011-11-09
 */

/**
 * Replaces the function that is called when the thread runs.
 */
INLINE void GenericThread::
set_function(std::function<void()> function) {
  _function = std::move(function);
}

/**
 * Returns the function that is called when the thread runs.
 */
INLINE const std::function<void()> &GenericThread::
get_function() const {
  return _function;
}
