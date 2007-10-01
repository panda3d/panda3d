// Filename: pointerEvent.cxx
// Created by:  drose (01Mar00)
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

#include "pointerEvent.h"
#include "datagram.h"
#include "datagramIterator.h"

////////////////////////////////////////////////////////////////////
//     Function: PointerEvent::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PointerEvent::
output(ostream &out) const {
  out << (_in_window ? "In@" : "Out@")
      << _xpos << "," << _ypos << " ";
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEvent::write_datagram
//       Access: Public
//  Description: Writes the event into a datagram.
////////////////////////////////////////////////////////////////////
void PointerEvent::
write_datagram(Datagram &dg) const {
  nassertv(false && "This function not implemented yet.");
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEvent::read_datagram
//       Access: Public
//  Description: Restores the event from the datagram.
////////////////////////////////////////////////////////////////////
void PointerEvent::
read_datagram(DatagramIterator &scan) {
  nassertv(false && "This function not implemented yet.");
}
