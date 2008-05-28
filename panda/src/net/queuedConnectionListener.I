// Filename: queuedConnectionListener.I
// Created by:  drose (17May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: ConnectionListenerData::operator ==
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ConnectionListenerData::
operator == (const ConnectionListenerData &) const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionListenerData::operator !=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ConnectionListenerData::
operator != (const ConnectionListenerData &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionListenerData::operator <
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ConnectionListenerData::
operator < (const ConnectionListenerData &) const {
  return false;
}
