/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileMountRamdisk.I
 * @author drose
 * @date 2011-09-19
 */

/**
 *
 */
INLINE VirtualFileMountRamdisk::FileBase::
FileBase(const std::string &basename) : _basename(basename), _timestamp(time(nullptr)) {
}

/**
 *
 */
INLINE bool VirtualFileMountRamdisk::FileBase::
operator < (const FileBase &other) const {
  return _basename < other._basename;
}

/**
 *
 */
INLINE VirtualFileMountRamdisk::File::
File(const std::string &basename) :
  FileBase(basename),
  _wrapper(&_data, false, true)
{
}

/**
 *
 */
INLINE VirtualFileMountRamdisk::Directory::
Directory(const std::string &basename) : FileBase(basename) {
}
