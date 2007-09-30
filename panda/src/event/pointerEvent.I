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
  _device(-1),
  _sequence(0),
  _time(0.0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEvent::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PointerEvent::
PointerEvent(int dev, const MouseData &data, int seq, double time) :
  _device(dev),
  _data(data),
  _sequence(seq),
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
  _device(copy._device),
  _data(copy._data),
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
  _device = copy._device;
  _data = copy._data;
  _sequence = copy._sequence;
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
  return (_device == other._device &&
          _data._xpos == other._data._xpos &&
          _data._ypos == other._data._ypos &&
          _sequence == other._sequence &&
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
  if (_device != other._device) {
    return _device < other._device;
  }
  if (_sequence != other._sequence) {
    return _sequence < other._sequence;
  }
  if (_data._xpos != other._data._xpos) {
    return _data._xpos < other._data._xpos;
  }
  if (_data._ypos != other._data._ypos) {
    return _data._ypos < other._data._ypos;
  }
  return _data._in_window < other._data._in_window;
}

