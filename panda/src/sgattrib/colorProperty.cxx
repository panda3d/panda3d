// Filename: colorProperty.cxx
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

#include "colorProperty.h"
#include <datagram.h>
#include <datagramIterator.h>

////////////////////////////////////////////////////////////////////
//     Function: ColorProperty::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ColorProperty::
output(ostream &out) const {
  if (is_real()) {
    out << "rgba(" << get_color() << ")";
  } else {
    out << "uncolor";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ColorProperty::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               this object to a Datagram
////////////////////////////////////////////////////////////////////
void ColorProperty::
write_datagram(Datagram &destination) {
  _color.write_datagram(destination);
  destination.add_bool(_real);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorProperty::read_datagram
//       Access: Public
//  Description: Function to write the important information into
//               this object out of a Datagram
////////////////////////////////////////////////////////////////////
void ColorProperty::
read_datagram(DatagramIterator &source) {
  _color.read_datagram(source);
  _real = source.get_bool();
}
