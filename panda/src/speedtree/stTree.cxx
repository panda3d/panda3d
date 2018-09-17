/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stTree.cxx
 * @author drose
 * @date 2010-10-06
 */

#include "stTree.h"
#include "speedTreeNode.h"

TypeHandle STTree::_type_handle;

/**
 * The constructor reads the indicated SRT file immediately.  Check is_valid()
 * to determine whether the read was successful or not.  Note that the
 * filename must be a fully-qualified pathname; the STTree constructor does
 * not search the model-path.
 */
STTree::
STTree(const Filename &fullpath) :
  Namable(fullpath.get_basename_wo_extension()),
  _fullpath(fullpath)
{
  _is_valid = false;

  // Ensure we have a license.
  if (!SpeedTreeNode::authorize()) {
    speedtree_cat.warning()
      << "SpeedTree license not available.\n";
    return;
  }

  // Can't use VFS, due to SpeedTree's insistence on using fopen() to load dds
  // textures and such.  So we go ahead and use the low-level Filename
  // interface directly.
  /*
  Filename tree_filename = filename;
  if (!tree_filename.resolve_filename(get_model_path(), "srt")) {
    speedtree_cat.warning()
      << "Couldn't find: " << filename << "\n";
    return false;
  }
  */

  std::string os_fullpath = _fullpath.to_os_specific();
  if (!_tree.LoadTree(os_fullpath.c_str())) {
    speedtree_cat.warning()
      << "Couldn't read: " << _fullpath << "\n";
    SpeedTreeNode::write_error(speedtree_cat.warning());
    return;
  }

  speedtree_cat.info()
    << "Read " << _fullpath << "\n";
  _is_valid = true;
}

/**
 *
 */
void STTree::
output(std::ostream &out) const {
  if (!is_valid()) {
    out << "(invalid STTree)";
  } else {
    out << "STTree(" << get_name() << ")";
  }
}
