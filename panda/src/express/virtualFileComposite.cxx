/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileComposite.cxx
 * @author drose
 * @date 2002-08-03
 */

#include "virtualFileComposite.h"

TypeHandle VirtualFileComposite::_type_handle;


/**
 * Returns the VirtualFileSystem this file is associated with.
 */
VirtualFileSystem *VirtualFileComposite::
get_file_system() const {
  return _file_system;
}

/**
 * Returns the full pathname to this file within the virtual file system.
 */
Filename VirtualFileComposite::
get_filename() const {
  return _filename;
}

/**
 * Returns true if this file exists, false otherwise.
 */
bool VirtualFileComposite::
has_file() const {
  return true;
}

/**
 * Returns true if this file represents a directory (and scan_directory() may
 * be called), false otherwise.
 */
bool VirtualFileComposite::
is_directory() const {
  return true;
}

/**
 * Fills file_list up with the list of files that are within this directory,
 * excluding those whose basenames are listed in mount_points.  Returns true
 * if successful, false if the file is not a directory or the directory cannot
 * be read.
 */
bool VirtualFileComposite::
scan_local_directory(VirtualFileList *file_list,
                     const ov_set<std::string> &mount_points) const {
  bool any_ok = false;
  Components::const_iterator ci;
  for (ci = _components.begin(); ci != _components.end(); ++ci) {
    if ((*ci)->scan_local_directory(file_list, mount_points)) {
      any_ok = true;
    }
  }

  return any_ok;
}
