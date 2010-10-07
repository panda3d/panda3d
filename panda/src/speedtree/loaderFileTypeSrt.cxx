// Filename: loaderFileTypeSrt.cxx
// Created by:  drose (06Oct10)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "loaderFileTypeSrt.h"
#include "speedTreeNode.h"
#include "stTree.h"

TypeHandle LoaderFileTypeSrt::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeSrt::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LoaderFileTypeSrt::
LoaderFileTypeSrt() {
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeSrt::get_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string LoaderFileTypeSrt::
get_name() const {
  return "SpeedTree compiled tree";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeSrt::get_extension
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string LoaderFileTypeSrt::
get_extension() const {
  return "srt";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeSrt::supports_compressed
//       Access: Published, Virtual
//  Description: Returns true if this file type can transparently load
//               compressed files (with a .pz extension), false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool LoaderFileTypeSrt::
supports_compressed() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeSrt::load_file
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(PandaNode) LoaderFileTypeSrt::
load_file(const Filename &path, const LoaderOptions &, 
          BamCacheRecord *record) const {
  if (!path.is_regular_file()) {
    // Quietly fail if the file doesn't exist.  The Loader expects
    // this.
    return NULL;
  }

  PT(STTree) tree = new STTree(path);
  if (!tree->is_valid()) {
    return NULL;
  }

  PT(SpeedTreeNode) st = new SpeedTreeNode(path.get_basename());
  st->add_instance(tree, STTransform());

  return st.p();
}
