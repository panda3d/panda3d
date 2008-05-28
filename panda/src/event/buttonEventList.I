// Filename: buttonEventList.I
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
//     Function: ButtonEventList::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonEventList::
ButtonEventList() {
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventList::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ButtonEventList::
ButtonEventList(const ButtonEventList &copy) :
  _events(copy._events)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventList::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ButtonEventList::
operator = (const ButtonEventList &copy) {
  _events = copy._events;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventList::add_event
//       Access: Public
//  Description: Adds a new event to the end of the list.
////////////////////////////////////////////////////////////////////
INLINE void ButtonEventList::
add_event(ButtonEvent event) {
  _events.push_back(event);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventList::get_num_events
//       Access: Public
//  Description: Returns the number of events in the list.
////////////////////////////////////////////////////////////////////
INLINE int ButtonEventList::
get_num_events() const {
  return _events.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventList::get_event
//       Access: Public
//  Description: Returns the nth event in the list.  This does not
//               remove the event from the list; the only way to
//               remove events is to empty the whole list with
//               clear().
////////////////////////////////////////////////////////////////////
INLINE const ButtonEvent &ButtonEventList::
get_event(int n) const {
#ifndef NDEBUG
  static ButtonEvent empty_event;
  nassertr(n >= 0 && n < (int)_events.size(), empty_event);
#endif  // NDEBUG
  return _events[n];
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventList::clear
//       Access: Public
//  Description: Empties all the events from the list.
////////////////////////////////////////////////////////////////////
INLINE void ButtonEventList::
clear() {
  _events.clear();
}
