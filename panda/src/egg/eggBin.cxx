// Filename: eggBin.cxx
// Created by:  drose (21Jan99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "eggBin.h"


TypeHandle EggBin::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggBin::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggBin::
EggBin(const string &name) : EggGroup(name) {
  _bin_number = 0;
}


////////////////////////////////////////////////////////////////////
//     Function: EggBin::EggGroup copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggBin::
EggBin(const EggGroup &copy) : EggGroup(copy) {
  _bin_number = 0;
}


////////////////////////////////////////////////////////////////////
//     Function: EggBin::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggBin::
EggBin(const EggBin &copy) : EggGroup(copy), _bin_number(copy._bin_number) {
}



////////////////////////////////////////////////////////////////////
//     Function: EggBin::set_bin_number
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggBin::
set_bin_number(int bin_number) {
  _bin_number = bin_number;
}

////////////////////////////////////////////////////////////////////
//     Function: EggBin::get_bin_number
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int EggBin::
get_bin_number() const {
  return _bin_number;
}
