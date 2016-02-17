/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file windowHandle.I
 * @author drose
 * @date 2009-09-30
 */

/**
 *
 */
INLINE WindowHandle::
WindowHandle(OSHandle *os_handle) : _os_handle(os_handle) {
}

/**
 *
 */
INLINE WindowHandle::
WindowHandle(const WindowHandle &copy) : _os_handle(copy._os_handle) {
}

/**
 * Returns the OS-specific handle stored internally to the WindowHandle
 * wrapper.
 */
INLINE WindowHandle::OSHandle *WindowHandle::
get_os_handle() const {
  return _os_handle;
}

/**
 * Changes the OS-specific handle stored internally to the WindowHandle
 * wrapper.
 */
INLINE void WindowHandle::
set_os_handle(OSHandle *os_handle) {
  _os_handle = os_handle;
}


/**
 * The base class of OSHandle doesn't have a usable constructor.  Always
 * construct an instance of some specialized type, that stores the appropriate
 * kind of window handle for each OS.
 */
INLINE WindowHandle::OSHandle::
OSHandle() {
}
