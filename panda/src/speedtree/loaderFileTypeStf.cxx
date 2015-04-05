// Filename: loaderFileTypeStf.cxx
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

#include "loaderFileTypeStf.h"
#include "speedTreeNode.h"

TypeHandle LoaderFileTypeStf::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeStf::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LoaderFileTypeStf::
LoaderFileTypeStf() {
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeStf::get_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string LoaderFileTypeStf::
get_name() const {
  return "SpeedTree compiled tree";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeStf::get_extension
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string LoaderFileTypeStf::
get_extension() const {
  return "stf";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeStf::supports_compressed
//       Access: Published, Virtual
//  Description: Returns true if this file type can transparently load
//               compressed files (with a .pz extension), false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool LoaderFileTypeStf::
supports_compressed() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeStf::load_file
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(PandaNode) LoaderFileTypeStf::
load_file(const Filename &path, const LoaderOptions &options, 
          BamCacheRecord *record) const {
  if (!path.is_regular_file()) {
    // Quietly fail if the file doesn't exist.  The Loader expects
    // this.
    return NULL;
  }

  PT(SpeedTreeNode) st = new SpeedTreeNode(path.get_basename());
  st->add_from_stf(path, options);

  return st.p();
}
