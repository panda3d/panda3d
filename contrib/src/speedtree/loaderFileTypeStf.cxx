/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderFileTypeStf.cxx
 * @author drose
 * @date 2010-10-06
 */

#include "loaderFileTypeStf.h"
#include "speedTreeNode.h"

TypeHandle LoaderFileTypeStf::_type_handle;

/**
 *
 */
LoaderFileTypeStf::
LoaderFileTypeStf() {
}

/**
 *
 */
std::string LoaderFileTypeStf::
get_name() const {
  return "SpeedTree compiled tree";
}

/**
 *
 */
std::string LoaderFileTypeStf::
get_extension() const {
  return "stf";
}

/**
 * Returns true if this file type can transparently load compressed files
 * (with a .pz or .gz extension), false otherwise.
 */
bool LoaderFileTypeStf::
supports_compressed() const {
  return true;
}

/**
 *
 */
PT(PandaNode) LoaderFileTypeStf::
load_file(const Filename &path, const LoaderOptions &options,
          BamCacheRecord *record) const {
  if (!path.is_regular_file()) {
    // Quietly fail if the file doesn't exist.  The Loader expects this.
    return nullptr;
  }

  PT(SpeedTreeNode) st = new SpeedTreeNode(path.get_basename());
  st->add_from_stf(path, options);

  return st;
}
