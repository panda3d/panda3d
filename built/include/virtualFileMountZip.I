/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileMountZip.I
 * @author rdb
 * @date 2019-11-07
 */

/**
 *
 */
INLINE VirtualFileMountZip::
VirtualFileMountZip(ZipArchive *archive, const Filename &directory) :
  _archive(archive),
  _directory(directory)
{
}

/**
 * Returns the ZipArchive pointer that this mount object is based on.
 */
INLINE ZipArchive *VirtualFileMountZip::
get_archive() const {
  return _archive;
}
