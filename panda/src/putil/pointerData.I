/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerData.I
 * @author drose
 * @date 2002-07-15
 */

/**
 *
 */
INLINE double PointerData::
get_x() const {
  return _xpos;
}

/**
 *
 */
INLINE double PointerData::
get_y() const {
  return _ypos;
}

/**
 * Returns a unique identifier for this pointer.  This is for tracking
 * individual fingers.  This value should not be assumed to have a specific
 * meaning other than that there will not be two different pointers active
 * simultaneously with the same identifier.
 */
INLINE int PointerData::
get_id() const {
  return _id;
}

/**
 * Returns the type of pointing device.
 */
INLINE PointerType PointerData::
get_type() const {
  return _type;
}

INLINE bool PointerData::
get_primary() const {
  return _primary;
}

/**
 * Returns the pressure of the pointer.  For mice, this will be 1.0 if any
 * button is pressed, 0.0 otherwise.
 */
INLINE double PointerData::
get_pressure() const {
  return _pressure;
}

INLINE PointerPhase PointerData::
get_phase() const {
  return _phase;
}
INLINE void PointerData::
set_pressure(double pressure) {
  _pressure = pressure;
}

INLINE void PointerData::
set_phase(PointerPhase phase) {
  _phase = phase;
}

INLINE void PointerData::
update(double x, double y, double pressure, PointerPhase phase) {
  _xpos = x;
  _ypos = y;
  _pressure = pressure;
  _phase = phase;
}

INLINE void PointerData::
rel_move(double rx, double ry) {
  _xpos += rx;
  _ypos += ry;
}

INLINE std::ostream &operator << (std::ostream &out, const PointerData &md) {
  md.output(out);
  return out;
}
