/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileMount.I
 * @author drose
 * @date 2002-08-03
 */

/**
 *
 */
INLINE VirtualFileMount::
VirtualFileMount() :
  _file_system(nullptr),
  _mount_flags(0)
{
}

/**
 * Returns the file system this mount object is attached to.
 */
INLINE VirtualFileSystem *VirtualFileMount::
get_file_system() const {
  return _file_system;
}


/**
 * Returns the name of the directory within the virtual file system that this
 * mount object is attached to.  This directory name will end with a slash.
 */
INLINE const Filename &VirtualFileMount::
get_mount_point() const {
  return _mount_point;
}

/**
 * Returns the set of flags passed by the user to the
 * VirtualFileSystem::mount() command.
 */
INLINE int VirtualFileMount::
get_mount_flags() const {
  return _mount_flags;
}


INLINE std::ostream &
operator << (std::ostream &out, const VirtualFileMount &mount) {
  mount.output(out);
  return out;
}
