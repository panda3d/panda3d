// Filename: qpdialNode.I
// Created by:  drose (12Mar02)
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


////////////////////////////////////////////////////////////////////
//     Function: qpDialNode::is_valid
//       Access: Public
//  Description: Returns true if the qpDialNode is valid and
//               connected to a server, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpDialNode::
is_valid() const {
  return (_dial != (ClientDialDevice *)NULL) && _dial->is_connected();
}

////////////////////////////////////////////////////////////////////
//     Function: qpDialNode::get_num_dials
//       Access: Public
//  Description: Returns the number of dial dials known to the
//               qpDialNode.  This number may change as more dials
//               are discovered.
////////////////////////////////////////////////////////////////////
INLINE int qpDialNode::
get_num_dials() const {
  _dial->lock();
  int result = _dial->get_num_dials();
  _dial->unlock();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: qpDialNode::read_dial
//       Access: Public
//  Description: Returns the number of complete revolutions of the
//               dial since the last time read_dial() was called.
//               This is a destructive operation; it is not possible
//               to read the dial without resetting the counter.
////////////////////////////////////////////////////////////////////
INLINE double qpDialNode::
read_dial(int index) {
  _dial->lock();
  double result = _dial->read_dial(index);
  _dial->unlock();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: qpDialNode::is_dial_known
//       Access: Public
//  Description: Returns true if the state of the indicated dial
//               dial is known, or false if we have never heard
//               anything about this particular dial.
////////////////////////////////////////////////////////////////////
INLINE bool qpDialNode::
is_dial_known(int index) const {
  _dial->lock();
  bool result = _dial->is_dial_known(index);
  _dial->unlock();
  return result;
}
