/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileMountSystem.I
 * @author drose
 * @date 2002-08-03
 */

/**
 *
 */
INLINE VirtualFileMountSystem::
VirtualFileMountSystem(const Filename &physical_filename) :
  _physical_filename(physical_filename)
{
}

/**
 * Returns the name of the source file on the OS filesystem of the directory
 * or file that is mounted.
 */
INLINE const Filename &VirtualFileMountSystem::
get_physical_filename() const {
  return _physical_filename;
}
