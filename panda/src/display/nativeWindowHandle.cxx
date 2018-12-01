/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nativeWindowHandle.cxx
 * @author drose
 * @date 2009-09-30
 */

#include "nativeWindowHandle.h"

using std::ostream;

TypeHandle NativeWindowHandle::_type_handle;
TypeHandle NativeWindowHandle::IntHandle::_type_handle;
TypeHandle NativeWindowHandle::SubprocessHandle::_type_handle;

#if defined(HAVE_X11) && !defined(CPPPARSER)
TypeHandle NativeWindowHandle::X11Handle::_type_handle;
#endif
#if defined(WIN32) && !defined(CPPPARSER)
TypeHandle NativeWindowHandle::WinHandle::_type_handle;
#endif

/**
 * Constructs a new WindowHandle with an int value, which is understood to be
 * either an HWND or a Window, cast to int.  This method exists for the
 * convenience of Python, which likes to deal with ints; C++ code should use
 * one of the more specific make_x11() or make_win32() methods instead.
 */
PT(WindowHandle) NativeWindowHandle::
make_int(size_t window) {
  return new WindowHandle(new IntHandle(window));
}

/**
 * Constructs a new WindowHandle that references a SubprocessWindowBuffer read
 * in another process, with the named pipe filename that it uses for
 * communication.
 *
 * This is (at present, and maybe always) useful only on the OS X platform,
 * where parenting child windows is particularly problematic.
 */
PT(WindowHandle) NativeWindowHandle::
make_subprocess(const Filename &filename) {
  return new WindowHandle(new SubprocessHandle(filename));
}

#if defined(HAVE_X11) && !defined(CPPPARSER)
/**
 * Constructs a new WindowHandle that references an X11 window.
 */
PT(WindowHandle) NativeWindowHandle::
make_x11(X11_Window window) {
  return new WindowHandle(new X11Handle(window));
}
#endif  // HAVE_X11

#if defined(WIN32) && !defined(CPPPARSER)
/**
 * Constructs a new WindowHandle that references a window on Windows.
 */
PT(WindowHandle) NativeWindowHandle::
make_win(HWND window) {
  return new WindowHandle(new WinHandle(window));
}
#endif  // WIN32

/**
 * Returns the OS-specific handle converted to an integer, if this is possible
 * for the particular representation.  Returns 0 if it is not.
 */
size_t NativeWindowHandle::IntHandle::
get_int_handle() const {
  return _handle;
}

/**
 *
 */
void NativeWindowHandle::IntHandle::
output(ostream &out) const {
  out << "(" << _handle << ")";
}

/**
 *
 */
void NativeWindowHandle::SubprocessHandle::
output(ostream &out) const {
  out << "(" << _filename << ")";
}

#if defined(HAVE_X11) && !defined(CPPPARSER)
/**
 * Returns the OS-specific handle converted to an integer, if this is possible
 * for the particular representation.  Returns 0 if it is not.
 */
size_t NativeWindowHandle::X11Handle::
get_int_handle() const {
  return (size_t)_handle;
}
#endif  // HAVE_X11

#if defined(HAVE_X11) && !defined(CPPPARSER)
/**
 *
 */
void NativeWindowHandle::X11Handle::
output(ostream &out) const {
  out << _handle;
}
#endif  // HAVE_X11

#if defined(WIN32) && !defined(CPPPARSER)
/**
 * Returns the OS-specific handle converted to an integer, if this is possible
 * for the particular representation.  Returns 0 if it is not.
 */
size_t NativeWindowHandle::WinHandle::
get_int_handle() const {
  return (size_t)_handle;
}
#endif  // WIN32

#if defined(WIN32) && !defined(CPPPARSER)
/**
 *
 */
void NativeWindowHandle::WinHandle::
output(ostream &out) const {
  out << _handle;
}
#endif  // WIN32
