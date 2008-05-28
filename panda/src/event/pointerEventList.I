// Filename: pointerEventList.I
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
//     Function: PointerEventList::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PointerEventList::
PointerEventList() {
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PointerEventList::
PointerEventList(const PointerEventList &copy) :
  _events(copy._events)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PointerEventList::
operator = (const PointerEventList &copy) {
  _events = copy._events;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::get_num_events
//       Access: Published
//  Description: Returns the number of events in the list.
////////////////////////////////////////////////////////////////////
INLINE int PointerEventList::
get_num_events() const {
  return _events.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::get_in_window
//       Access: Published
//  Description: Get the in-window flag of the nth event.
////////////////////////////////////////////////////////////////////
INLINE bool PointerEventList::
get_in_window(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._in_window;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::get_xpos
//       Access: Published
//  Description: Get the x-coordinate of the nth event.
////////////////////////////////////////////////////////////////////
INLINE int PointerEventList::
get_xpos(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._xpos;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::get_ypos
//       Access: Published
//  Description: Get the y-coordinate of the nth event.
////////////////////////////////////////////////////////////////////
INLINE int PointerEventList::
get_ypos(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._ypos;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::get_dx
//       Access: Published
//  Description: Get the x-coordinate of the nth event.
////////////////////////////////////////////////////////////////////
INLINE int PointerEventList::
get_dx(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._dx;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::get_dy
//       Access: Published
//  Description: Get the y-coordinate of the nth event.
////////////////////////////////////////////////////////////////////
INLINE int PointerEventList::
get_dy(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._dy;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::get_length
//       Access: Published
//  Description: Get the length of the nth event.
////////////////////////////////////////////////////////////////////
INLINE double PointerEventList::
get_length(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._length;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::get_direction
//       Access: Published
//  Description: Get the direction of the nth event.
////////////////////////////////////////////////////////////////////
INLINE double PointerEventList::
get_direction(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._direction;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::get_rotation
//       Access: Published
//  Description: Get the rotation of the nth event.
////////////////////////////////////////////////////////////////////
INLINE double PointerEventList::
get_rotation(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._rotation;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::get_sequence
//       Access: Published
//  Description: Get the sequence number of the nth event.
////////////////////////////////////////////////////////////////////
INLINE int PointerEventList::
get_sequence(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._sequence;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::get_time
//       Access: Published
//  Description: Get the timestamp of the nth event.
////////////////////////////////////////////////////////////////////
INLINE double PointerEventList::
get_time(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._time;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::clear
//       Access: Public
//  Description: Empties all the events from the list.
////////////////////////////////////////////////////////////////////
INLINE void PointerEventList::
clear() {
  _events.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::pop_front
//       Access: Public
//  Description: Discards the first event on the list.
////////////////////////////////////////////////////////////////////
INLINE void PointerEventList::
pop_front() {
  _events.pop_front();
}
