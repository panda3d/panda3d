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
//       Access: Public
//  Description: Returns the number of events in the list.
////////////////////////////////////////////////////////////////////
INLINE int PointerEventList::
get_num_events() const {
  return _events.size();
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
