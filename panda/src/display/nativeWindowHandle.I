/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nativeWindowHandle.I
 * @author drose
 * @date 2009-09-30
 */

/**
 *
 */
INLINE NativeWindowHandle::IntHandle::
IntHandle(size_t handle) : _handle(handle) {
}

/**
 *
 */
INLINE size_t NativeWindowHandle::IntHandle::
get_handle() const {
  return _handle;
}

/**
 *
 */
INLINE NativeWindowHandle::SubprocessHandle::
SubprocessHandle(const Filename &filename) : _filename(filename) {
}

/**
 *
 */
INLINE const Filename &NativeWindowHandle::SubprocessHandle::
get_filename() const {
  return _filename;
}

#if defined(HAVE_X11) && !defined(CPPPARSER)
/**
 *
 */
INLINE NativeWindowHandle::X11Handle::
X11Handle(X11_Window handle) : _handle(handle) {
}
#endif  // HAVE_X11

#if defined(HAVE_X11) && !defined(CPPPARSER)
/**
 *
 */
INLINE X11_Window NativeWindowHandle::X11Handle::
get_handle() const {
  return _handle;
}
#endif  // HAVE_X11

#if defined(WIN32) && !defined(CPPPARSER)
/**
 *
 */
INLINE NativeWindowHandle::WinHandle::
WinHandle(HWND handle) : _handle(handle) {
}
#endif  // WIN32

#if defined(WIN32) && !defined(CPPPARSER)
/**
 *
 */
INLINE HWND NativeWindowHandle::WinHandle::
get_handle() const {
  return _handle;
}
#endif  // WIN32
