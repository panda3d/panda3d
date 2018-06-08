/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileSimple.I
 * @author drose
 * @date 2002-08-03
 */

/**
 *
 */
INLINE VirtualFileSimple::
VirtualFileSimple(VirtualFileMount *mount, const Filename &local_filename,
                  bool implicit_pz_file, int open_flags) :
  _mount(mount),
  _local_filename(local_filename),
  _implicit_pz_file(implicit_pz_file)
{
}

/**
 * Returns the VirtualFileMount this file is associated with.
 */
INLINE VirtualFileMount *VirtualFileSimple::
get_mount() const {
  return _mount;
}

/**
 * Returns true if this file is a .pz file that should be implicitly
 * decompressed on load, or false if it is not a .pz file or if it should not
 * be decompressed.
 */
INLINE bool VirtualFileSimple::
is_implicit_pz_file() const {
  return _implicit_pz_file;
}
