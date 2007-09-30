// Filename: pointerEventList.I
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
//     Function: PointerEventList::add_event
//       Access: Public
//  Description: Adds a new event to the end of the list.
////////////////////////////////////////////////////////////////////
INLINE void PointerEventList::
add_event(const PointerEvent &event) {
  _events.push_back(event);
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
//     Function: PointerEventList::get_x
//       Access: Published
//  Description: Get the x-coordinate of the nth event.
////////////////////////////////////////////////////////////////////
INLINE int PointerEventList::
get_x(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._data._xpos;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::get_y
//       Access: Published
//  Description: Get the y-coordinate of the nth event.
////////////////////////////////////////////////////////////////////
INLINE int PointerEventList::
get_y(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._data._ypos;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerEventList::get_in_window
//       Access: Published
//  Description: Get the in-window flag of the nth event.
////////////////////////////////////////////////////////////////////
INLINE bool PointerEventList::
get_in_window(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._data._in_window;
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
//     Function: PointerEventList::get_event
//       Access: Public
//  Description: Returns the nth event in the list.  This does not
//               remove the event from the list; the only way to
//               remove events is to empty the whole list with
//               clear().
////////////////////////////////////////////////////////////////////
INLINE const PointerEvent &PointerEventList::
get_event(int n) const {
#ifndef NDEBUG
  static PointerEvent empty_event;
  nassertr(n >= 0 && n < (int)_events.size(), empty_event);
#endif  // NDEBUG
  return _events[n];
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
