// Filename: dialNode.I
// Created by:  drose (12Mar02)
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
//     Function: DialNode::is_valid
//       Access: Public
//  Description: Returns true if the DialNode is valid and
//               connected to a server, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool DialNode::
is_valid() const {
  return (_dial != (ClientDialDevice *)NULL) && _dial->is_connected();
}

////////////////////////////////////////////////////////////////////
//     Function: DialNode::get_num_dials
//       Access: Public
//  Description: Returns the number of dial dials known to the
//               DialNode.  This number may change as more dials
//               are discovered.
////////////////////////////////////////////////////////////////////
INLINE int DialNode::
get_num_dials() const {
  _dial->acquire();
  int result = _dial->get_num_dials();
  _dial->unlock();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: DialNode::read_dial
//       Access: Public
//  Description: Returns the number of complete revolutions of the
//               dial since the last time read_dial() was called.
//               This is a destructive operation; it is not possible
//               to read the dial without resetting the counter.
////////////////////////////////////////////////////////////////////
INLINE double DialNode::
read_dial(int index) {
  _dial->acquire();
  double result = _dial->read_dial(index);
  _dial->unlock();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: DialNode::is_dial_known
//       Access: Public
//  Description: Returns true if the state of the indicated dial
//               dial is known, or false if we have never heard
//               anything about this particular dial.
////////////////////////////////////////////////////////////////////
INLINE bool DialNode::
is_dial_known(int index) const {
  _dial->acquire();
  bool result = _dial->is_dial_known(index);
  _dial->unlock();
  return result;
}
