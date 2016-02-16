/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file netDatagram.I
 * @author drose
 * @date 2000-05-17
 */

////////////////////////////////////////////////////////////////////
//     Function: NetDatagram::operator ==
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool NetDatagram::
operator == (const NetDatagram &) const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NetDatagram::operator !=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool NetDatagram::
operator != (const NetDatagram &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NetDatagram::operator <
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool NetDatagram::
operator < (const NetDatagram &) const {
  return false;
}
