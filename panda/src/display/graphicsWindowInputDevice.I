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
 * only makes sense if has_pointer() also returns true.
 */
INLINE PointerData GraphicsWindowInputDevice::
get_pointer() const {
  LightMutexHolder holder(_lock);
  if (!_pointers.empty()) {
    return _pointers[0];
  } else {
    return PointerData();
  }
}

/**
 * To be called by a particular kind of GraphicsWindow to indicate that the
 * pointer data has changed.
 */
INLINE void GraphicsWindowInputDevice::
update_pointer(PointerData data, double time) {
  LightMutexHolder holder(_lock);
  InputDevice::update_pointer(std::move(data), time);
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

/**
 * To be called by a particular kind of GraphicsWindow to indicate that the
 * pointer no longer exists.
 */
INLINE void GraphicsWindowInputDevice::
remove_pointer(int id) {
  LightMutexHolder holder(_lock);
  InputDevice::remove_pointer(id);
}
