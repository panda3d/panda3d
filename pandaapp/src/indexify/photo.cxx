// Filename: photo.cxx
// Created by:  drose (03Apr02)
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

#include "photo.h"


////////////////////////////////////////////////////////////////////
//     Function: Photo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Photo::
Photo(RollDirectory *dir, const Filename &basename) :
  _dir(dir),
  _basename(basename)
{
  _name = _basename.get_basename_wo_extension();
  _full_x_size = 0;
  _full_y_size = 0;
  _reduced_x_size = 0;
  _reduced_y_size = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Photo::get_basename
//       Access: Public
//  Description: Returns the filename of the photo within its roll
//               directory.
////////////////////////////////////////////////////////////////////
const Filename &Photo::
get_basename() const {
  return _basename;
}

////////////////////////////////////////////////////////////////////
//     Function: Photo::get_name
//       Access: Public
//  Description: Returns the name of the photo without its filename
//               extension.
////////////////////////////////////////////////////////////////////
const string &Photo::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: Photo::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Photo::
output(ostream &out) const {
  out << _basename;
}
