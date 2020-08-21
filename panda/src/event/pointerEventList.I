/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerEventList.I
 * @author jyelon
 * @date 2007-09-20
 */

/**
 *
 */
INLINE PointerEventList::
PointerEventList() {
}

/**
 *
 */
INLINE PointerEventList::
PointerEventList(const PointerEventList &copy) :
  _events(copy._events)
{
}

/**
 *
 */
INLINE void PointerEventList::
operator = (const PointerEventList &copy) {
  _events = copy._events;
}

/**
 * Returns the number of events in the list.
 */
INLINE size_t PointerEventList::
get_num_events() const {
  return _events.size();
}

/**
 * Returns true if the list is empty.
 */
INLINE bool PointerEventList::
empty() const {
  return _events.empty();
}

/**
 * Returns the nth event in the list.
 */
INLINE const PointerEvent &PointerEventList::
get_event(size_t n) const {
  return _events[n];
}

/**
 * Get the in-window flag of the nth event.
 */
INLINE bool PointerEventList::
get_in_window(size_t n) const {
  nassertr(n < _events.size(), 0);
  return _events[n]._in_window;
}

/**
 * Get the x-coordinate of the nth event.
 */
INLINE int PointerEventList::
get_xpos(size_t n) const {
  nassertr(n < _events.size(), 0);
  return _events[n]._xpos;
}

/**
 * Get the y-coordinate of the nth event.
 */
INLINE int PointerEventList::
get_ypos(size_t n) const {
  nassertr(n < _events.size(), 0);
  return _events[n]._ypos;
}

/**
 * Get the x-delta of the nth event.
 */
INLINE double PointerEventList::
get_dx(size_t n) const {
  nassertr(n < _events.size(), 0);
  return _events[n]._dx;
}

/**
 * Get the y-delta of the nth event.
 */
INLINE double PointerEventList::
get_dy(size_t n) const {
  nassertr(n < _events.size(), 0);
  return _events[n]._dy;
}

/**
 * Get the length of the nth event.
 */
INLINE double PointerEventList::
get_length(size_t n) const {
  nassertr(n < _events.size(), 0);
  return _events[n]._length;
}

/**
 * Get the direction of the nth event.
 */
INLINE double PointerEventList::
get_direction(size_t n) const {
  nassertr(n < _events.size(), 0);
  return _events[n]._direction;
}

/**
 * Get the rotation of the nth event.
 */
INLINE double PointerEventList::
get_rotation(size_t n) const {
  nassertr(n < _events.size(), 0);
  return _events[n]._rotation;
}

/**
 * Get the sequence number of the nth event.
 */
INLINE int PointerEventList::
get_sequence(size_t n) const {
  nassertr(n < _events.size(), 0);
  return _events[n]._sequence;
}

/**
 * Get the timestamp of the nth event.
 */
INLINE double PointerEventList::
get_time(size_t n) const {
  nassertr(n < _events.size(), 0);
  return _events[n]._time;
}

/**
 * Empties all the events from the list.
 */
INLINE void PointerEventList::
clear() {
  _events.clear();
}

/**
 * Discards the first event on the list.
 */
INLINE void PointerEventList::
pop_front() {
  _events.pop_front();
}
