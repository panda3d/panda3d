// Filename: dialNode.I
// Created by:  drose (12Mar02)
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
  _dial->lock();
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
  _dial->lock();
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
  _dial->lock();
  bool result = _dial->is_dial_known(index);
  _dial->unlock();
  return result;
}
