// Filename: depthTestProperty.cxx
// Created by:  drose (22Mar00)
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

#include "depthTestProperty.h"
#include <datagram.h>
#include <datagramIterator.h>

ostream &
operator << (ostream &out, DepthTestProperty::Mode mode) {
  switch (mode) {
  case DepthTestProperty::M_none:
    return out << "none";

  case DepthTestProperty::M_never:
    return out << "never";

  case DepthTestProperty::M_less:
    return out << "less";

  case DepthTestProperty::M_equal:
    return out << "equal";

  case DepthTestProperty::M_less_equal:
    return out << "less_equal";

  case DepthTestProperty::M_greater:
    return out << "greater";

  case DepthTestProperty::M_not_equal:
    return out << "not_equal";

  case DepthTestProperty::M_greater_equal:
    return out << "greater_equal";

  case DepthTestProperty::M_always:
    return out << "always";
  }

  return out << "**invalid**(" << (int)mode << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestProperty::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               this object to a Datagram
////////////////////////////////////////////////////////////////////
void DepthTestProperty::
write_datagram(Datagram &destination) {
  destination.add_uint8(_mode);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestProperty::read_datagram
//       Access: Public
//  Description: Function to write the important information into
//               this object out of a Datagram
////////////////////////////////////////////////////////////////////
void DepthTestProperty::
read_datagram(DatagramIterator &source) {
  _mode = (Mode)source.get_uint8();
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestProperty::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DepthTestProperty::
output(ostream &out) const {
  out << _mode;
}
