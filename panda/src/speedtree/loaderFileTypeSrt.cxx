/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderFileTypeSrt.cxx
 * @author drose
 * @date 2010-10-06
 */

#include "loaderFileTypeSrt.h"
#include "speedTreeNode.h"
#include "stTree.h"

TypeHandle LoaderFileTypeSrt::_type_handle;

/**
 *
 */
LoaderFileTypeSrt::
LoaderFileTypeSrt() {
}

/**
 *
 */
std::string LoaderFileTypeSrt::
get_name() const {
  return "SpeedTree compiled tree";
}

/**
 *
 */
std::string LoaderFileTypeSrt::
get_extension() const {
  return "srt";
}

/**
 * Returns true if this file type can transparently load compressed files
 * (with a .pz or .gz extension), false otherwise.
 */
bool LoaderFileTypeSrt::
supports_compressed() const {
  return false;
}

/**
 *
 */
PT(PandaNode) LoaderFileTypeSrt::
load_file(const Filename &path, const LoaderOptions &,
          BamCacheRecord *record) const {
  if (!path.is_regular_file()) {
    // Quietly fail if the file doesn't exist.  The Loader expects this.
    return nullptr;
  }

  PT(STTree) tree = new STTree(path);
  if (!tree->is_valid()) {
    return nullptr;
  }

  PT(SpeedTreeNode) st = new SpeedTreeNode(path.get_basename());
  st->add_instance(tree, STTransform());

  return st;
}
