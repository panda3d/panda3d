/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggBin.cxx
 * @author drose
 * @date 1999-01-21
 */

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
