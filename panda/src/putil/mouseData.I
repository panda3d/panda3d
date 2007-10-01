// Filename: mouseData.I
// Created by:  drose (15Jul02)
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
//     Function: MouseData::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MouseData::
MouseData() {
  _in_window = false;
  _xpos = 0;
  _ypos = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseData::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MouseData::
MouseData(const MouseData &copy) :
  _in_window(copy._in_window),
  _xpos(copy._xpos),
  _ypos(copy._ypos)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MouseData::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void MouseData::
operator = (const MouseData &copy) {
  _in_window = copy._in_window;
  _xpos = copy._xpos;
  _ypos = copy._ypos;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseData::get_x
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int MouseData::
get_x() const {
  return _xpos;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseData::get_y
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int MouseData::
get_y() const {
  return _ypos;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseData::get_in_window
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool MouseData::
get_in_window() const {
  return _in_window;
}


INLINE ostream &operator << (ostream &out, const MouseData &md) {
  md.output(out);
  return out;
}
