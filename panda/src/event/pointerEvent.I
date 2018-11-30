/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerEvent.I
 * @author jyelon
 * @date 2007-09-20
 */

/**
 * The equality operator does not consider time significant.
 */
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

/**
 *
 */
INLINE bool PointerEvent::
operator != (const PointerEvent &other) const {
  return !operator == (other);
}

/**
 *
 */
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
