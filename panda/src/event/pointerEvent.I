// Filename: pointerEvent.I
// Created by: jyelon (20Sep2007)
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
//     Function: PointerEvent::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PointerEvent::
PointerEvent() :
  _in_window(false),
  _xpos(0),
  _ypos(0),
  _dx(0),
  _dy(0),
  _length(0.0),
  _direction(0.0),
  _rotation(0.0),
  _sequence(0),
  _time(0.0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEvent::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PointerEvent::
PointerEvent(const PointerEvent &copy) :
  _in_window(copy._in_window),
  _xpos(copy._xpos),
  _ypos(copy._ypos),
  _dx(copy._dx),
  _dy(copy._dy),
  _length(copy._length),
  _direction(copy._direction),
  _rotation(copy._rotation),
  _sequence(copy._sequence),
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
  _in_window = copy._in_window;
  _xpos = copy._xpos;
  _ypos = copy._ypos;
  _dx = copy._dx;
  _dy = copy._dy;
  _sequence = copy._sequence;
  _length = copy._length;
  _direction = copy._direction;
  _rotation = copy._rotation;
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
  return (_in_window == other._in_window &&
          _xpos == other._xpos &&
          _ypos == other._ypos &&
          _dx == other._dx &&
          _dy == other._dy &&
          _sequence == other._sequence &&
          _length == other._length &&
          _direction == other._direction &&
          _rotation == other._rotation);
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
  if (_sequence != other._sequence) {
    return _sequence < other._sequence;
  }
  if (_xpos != other._xpos) {
    return _xpos < other._xpos;
  }
  if (_ypos != other._ypos) {
    return _ypos < other._ypos;
  }
  if (_dx != other._dx) {
    return _dx < other._dx;
  }
  if (_dy != other._dy) {
    return _dy < other._dy;
  }
  if (_length != other._length) {
    return _length < other._length;
  }
  if (_direction != other._direction) {
    return _direction < other._direction;
  }
  if (_rotation != other._rotation) {
    return _rotation < other._rotation;
  }
  return _in_window < other._in_window;
}

