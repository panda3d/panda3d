// Filename: event.I
// Created by:  drose (15May01)
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
//     Function: Event::set_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Event::
set_name(const string &name) {
  _name = name;
}

////////////////////////////////////////////////////////////////////
//     Function: Event::clear_name
//       Access: Public
//  Description: Resets the Event's name to empty.
////////////////////////////////////////////////////////////////////
INLINE void Event::
clear_name() {
  _name = "";
}

////////////////////////////////////////////////////////////////////
//     Function: Event::has_name
//       Access: Public
//  Description: Returns true if the Event has a nonempty name set,
//               false if the name is empty.
////////////////////////////////////////////////////////////////////
INLINE bool Event::
has_name() const {
  return !_name.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: Event::get_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &Event::
get_name() const {
  return _name;
}


INLINE ostream &operator << (ostream &out, const Event &n) {
  n.output(out);
  return out;
}
