// Filename: loaderFileTypeEgg.cxx
// Created by:  drose (20Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "loaderFileTypeEgg.h"
#include "load_egg_file.h"

#include "eggData.h"

TypeHandle LoaderFileTypeEgg::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LoaderFileTypeEgg::
LoaderFileTypeEgg() {
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::get_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string LoaderFileTypeEgg::
get_name() const {
  return "Egg";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::get_extension
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string LoaderFileTypeEgg::
get_extension() const {
  return "egg";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::resolve_filename
//       Access: Public, Virtual
//  Description: Searches for the indicated filename on whatever paths
//               are appropriate to this file type, and updates it if
//               it is found.
////////////////////////////////////////////////////////////////////
void LoaderFileTypeEgg::
resolve_filename(Filename &path) const {
  EggData::resolve_egg_filename(path);
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::load_file
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(PandaNode) LoaderFileTypeEgg::
load_file(const Filename &path, bool) const {
  PT(PandaNode) result = load_egg_file(path);
  return result;
}
