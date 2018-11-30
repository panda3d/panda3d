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
 * To be called by a particular kind of GraphicsWindow to indicate that the
 * pointer is within the window, at the given pixel coordinates.
 */
INLINE void GraphicsWindowInputDevice::
set_pointer_in_window(double x, double y, double time) {
  LightMutexHolder holder(_lock);
  InputDevice::set_pointer(true, x, y, time);
}

/**
 * To be called by a particular kind of GraphicsWindow to indicate that the
 * pointer is no longer within the window.
 */
INLINE void GraphicsWindowInputDevice::
set_pointer_out_of_window(double time) {
  LightMutexHolder holder(_lock);
  InputDevice::set_pointer_out_of_window(time);
}

/**
 * To be called by a particular kind of GraphicsWindow to indicate that the
 * pointer has moved by the given relative amount.
 */
INLINE void GraphicsWindowInputDevice::
pointer_moved(double x, double y, double time) {
  LightMutexHolder holder(_lock);
  InputDevice::pointer_moved(x, y, time);
}
