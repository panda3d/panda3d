/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file buttonEventList.I
 * @author drose
 * @date 2002-03-12
 */

/**
 *
 */
INLINE ButtonEventList::
ButtonEventList() {
}

/**
 *
 */
INLINE ButtonEventList::
ButtonEventList(const ButtonEventList &copy) :
  _events(copy._events)
{
}

/**
 *
 */
INLINE void ButtonEventList::
operator = (const ButtonEventList &copy) {
  _events = copy._events;
}

/**
 * Adds a new event to the end of the list.
 */
INLINE void ButtonEventList::
add_event(ButtonEvent event) {
  _events.push_back(event);
}

/**
 * Returns the number of events in the list.
 */
INLINE int ButtonEventList::
get_num_events() const {
  return _events.size();
}

/**
 * Returns the nth event in the list.  This does not remove the event from the
 * list; the only way to remove events is to empty the whole list with
 * clear().
 */
INLINE const ButtonEvent &ButtonEventList::
get_event(int n) const {
#ifndef NDEBUG
  static ButtonEvent empty_event;
  nassertr(n >= 0 && n < (int)_events.size(), empty_event);
#endif  // NDEBUG
  return _events[n];
}

/**
 * Empties all the events from the list.
 */
INLINE void ButtonEventList::
clear() {
  _events.clear();
}
