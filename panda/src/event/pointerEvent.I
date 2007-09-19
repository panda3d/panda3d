// Filename: pointerEvent.I
// Created by: jyelon (20Sep2007)
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
//     Function: PointerEvent::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PointerEvent::
PointerEvent() :
  _pointer(-1),
  _time(0.0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEvent::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PointerEvent::
PointerEvent(int pointer, const MouseData &data, double time) :
  _pointer(pointer),
  _data(data),
  _time(time)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEvent::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PointerEvent::
PointerEvent(const PointerEvent &copy) :
  _pointer(copy._pointer),
  _data(copy._data),
  _time(copy._time)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEvent::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PointerEvent::
operator = (const PointerEvent &copy) {
  _pointer = copy._pointer;
  _data = copy._data;
  _time = copy._time;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEvent::Equality Operator
//       Access: Public
//  Description: The equality operator does not consider time
//               significant.
////////////////////////////////////////////////////////////////////
INLINE bool PointerEvent::
operator == (const PointerEvent &other) const {
  return (_pointer == other._pointer &&
          _data._xpos == other._data._xpos &&
          _data._ypos == other._data._ypos &&
          _data._in_window == other._data._in_window);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEvent::Inequality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool PointerEvent::
operator != (const PointerEvent &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEvent::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool PointerEvent::
operator < (const PointerEvent &other) const {
  if (_pointer != other._pointer) {
    return _pointer < other._pointer;
  }
  if (_data._xpos != other._data._xpos) {
    return _data._xpos < other._data._xpos;
  }
  if (_data._ypos != other._data._ypos) {
    return _data._ypos < other._data._ypos;
  }
  return _data._in_window < other._data._in_window;
}

