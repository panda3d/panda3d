/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomDrawCallbackData.I
 * @author drose
 * @date 2009-03-13
 */

/**
 *
 */
INLINE GeomDrawCallbackData::
GeomDrawCallbackData(CullableObject *obj, GraphicsStateGuardianBase *gsg,
                     bool force) :
  _obj(obj),
  _gsg(gsg),
  _force(force),
  _lost_state(true)
{
}

/**
 * Returns a pointer to the particular object that is being drawn.
 */
INLINE CullableObject *GeomDrawCallbackData::
get_object() const {
  return _obj;
}

/**
 * Returns a pointer to the current GSG.
 */
INLINE GraphicsStateGuardianBase *GeomDrawCallbackData::
get_gsg() const {
  return _gsg;
}

/**
 * Returns true if any required data should be forced into memory if necessary
 * to render the object, or false if the object should be omitted if some of
 * the data is not available (at least until the data becomes available
 * later).
 */
INLINE bool GeomDrawCallbackData::
get_force() const {
  return _force;
}

/**
 * Sets the lost_state flag.  If this is true, the callback does not have to
 * be quite so careful to clean up after itself; Panda will assume that the
 * graphics state is in an unknown state after the callback has finished, and
 * will issue all the necessary calls to restore it.  If this is false, Panda
 * will assume the callback will leave the graphics state exactly as it came
 * in, and won't bother to try to restore it.  The default is true.
 */
INLINE void GeomDrawCallbackData::
set_lost_state(bool lost_state) {
  _lost_state = lost_state;
}

/**
 * Returns the lost_state flag.  See set_lost_state().
 */
INLINE bool GeomDrawCallbackData::
get_lost_state() const {
  return _lost_state;
}
