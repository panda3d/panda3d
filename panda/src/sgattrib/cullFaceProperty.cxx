// Filename: cullFaceProperty.cxx
// Created by:  drose (23Mar00)
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

#include "cullFaceProperty.h"
#include <datagram.h>
#include <datagramIterator.h>

ostream &
operator << (ostream &out, CullFaceProperty::Mode mode) {
  switch (mode) {
  case CullFaceProperty::M_cull_none:
    return out << "cull_none";

  case CullFaceProperty::M_cull_clockwise:
    return out << "cull_clockwise";

  case CullFaceProperty::M_cull_counter_clockwise:
    return out << "cull_counter_clockwise";

  case CullFaceProperty::M_cull_all:
    return out << "cull_all";
  }

  return out << "**invalid**(" << (int)mode << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceProperty::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CullFaceProperty::
output(ostream &out) const {
  out << _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceProperty::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               this object to a Datagram
////////////////////////////////////////////////////////////////////
void CullFaceProperty::
write_datagram(Datagram &destination)
{
  destination.add_uint8(_mode);
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceProperty::read_datagram
//       Access: Public
//  Description: Function to write the important information into
//               this object out of a Datagram
////////////////////////////////////////////////////////////////////
void CullFaceProperty::
read_datagram(DatagramIterator &source)
{
  _mode = (enum Mode) source.get_uint8();
}
