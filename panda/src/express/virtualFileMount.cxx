// Filename: virtualFileMount.cxx
// Created by:  drose (03Aug02)
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

#include "virtualFileMount.h"

TypeHandle VirtualFileMount::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMount::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
VirtualFileMount::
~VirtualFileMount() {
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMount::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void VirtualFileMount::
output(ostream &out) const {
  out << get_physical_filename();
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMount::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void VirtualFileMount::
write(ostream &out) const {
  out << get_physical_filename() << " on /" << get_mount_point() << "\n";
}
