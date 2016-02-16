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

 */
INLINE PointerEventList::
PointerEventList() {
}

/**

 */
INLINE PointerEventList::
PointerEventList(const PointerEventList &copy) :
  _events(copy._events)
{
}

/**

 */
INLINE void PointerEventList::
operator = (const PointerEventList &copy) {
  _events = copy._events;
}

/**
 * Returns the number of events in the list.
 */
INLINE int PointerEventList::
get_num_events() const {
  return _events.size();
}

/**
 * Get the in-window flag of the nth event.
 */
INLINE bool PointerEventList::
get_in_window(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._in_window;
}

/**
 * Get the x-coordinate of the nth event.
 */
INLINE int PointerEventList::
get_xpos(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._xpos;
}

/**
 * Get the y-coordinate of the nth event.
 */
INLINE int PointerEventList::
get_ypos(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._ypos;
}

/**
 * Get the x-coordinate of the nth event.
 */
INLINE int PointerEventList::
get_dx(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._dx;
}

/**
 * Get the y-coordinate of the nth event.
 */
INLINE int PointerEventList::
get_dy(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._dy;
}

/**
 * Get the length of the nth event.
 */
INLINE double PointerEventList::
get_length(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._length;
}

/**
 * Get the direction of the nth event.
 */
INLINE double PointerEventList::
get_direction(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._direction;
}

/**
 * Get the rotation of the nth event.
 */
INLINE double PointerEventList::
get_rotation(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._rotation;
}

/**
 * Get the sequence number of the nth event.
 */
INLINE int PointerEventList::
get_sequence(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._sequence;
}

/**
 * Get the timestamp of the nth event.
 */
INLINE double PointerEventList::
get_time(int evt) const {
  nassertr((evt >= 0) && (evt < (int)_events.size()), 0);
  return _events[evt]._time;
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
