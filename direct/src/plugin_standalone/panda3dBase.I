/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file panda3dBase.I
 * @author rdb
 * @date 2009-12-07
 */

/**
 * Returns true if it is time to exit because the last instance has exited, or
 * false if we should continue running.
 */
bool Panda3DBase::
time_to_exit() {
  return _instances.empty() && _exit_with_last_instance;
}

/**
 * Returns the P3D_instance associated with this URLGetter.
 */
P3D_instance *Panda3DBase::URLGetter::
get_instance() {
  return _instance;
}
