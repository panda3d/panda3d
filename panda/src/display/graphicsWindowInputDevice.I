/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsWindowInputDevice.I
 * @author drose
 * @date 2000-05-24
 */

/**
 * Returns the PointerData associated with the input device's pointer.  This
 * only makes sense if has_pointer() also returns true. If you're using
 * multitouch, you should use get_primary_pointer() instead.
 *
 * @deprecated 1.11
 */
INLINE PointerData GraphicsWindowInputDevice::
get_pointer() const {
  LightMutexHolder holder(_lock);
  if (!_pointers.empty()) {
    return _pointers.begin()->second;
  } else {
    return PointerData();
  }
}

INLINE PointerData& GraphicsWindowInputDevice::
get_pointer(int id) {
  LightMutexHolder holder(_lock);
  return InputDevice::get_pointer(id);
}

INLINE PointerData& GraphicsWindowInputDevice::
add_pointer(PointerType type, int id, bool primary) {
  LightMutexHolder holder(_lock);
  return InputDevice::add_pointer(type, id, primary);
}

/**
 * To be called by a particular kind of GraphicsWindow to indicate that the
 * pointer has moved by the given relative amount.
 */
INLINE void GraphicsWindowInputDevice::
pointer_moved(double x, double y, double time) {
  LightMutexHolder holder(_lock);
  InputDevice::pointer_moved(0, x, y, time);
}

INLINE void GraphicsWindowInputDevice::
pointer_moved(int id, double x, double y, double time) {
  LightMutexHolder holder(_lock);
  InputDevice::pointer_moved(id, x, y, time);
}

/**
 * To be called by a particular kind of GraphicsWindow to indicate that the
 * pointer no longer exists.
 */
INLINE void GraphicsWindowInputDevice::
remove_pointer(int id) {
  LightMutexHolder holder(_lock);
  InputDevice::remove_pointer(id);
}

INLINE void GraphicsWindowInputDevice::
update_pointer(int id, double x, double y, double pressure, PointerPhase phase) {
  LightMutexHolder holder(_lock);
  InputDevice::update_pointer(id, x, y, pressure, phase);
}

INLINE bool GraphicsWindowInputDevice::
get_pointer_in_window() {
  LightMutexHolder holder(_lock);
  return _pointers.find(0) != _pointers.end();
}
