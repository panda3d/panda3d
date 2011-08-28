// Filename: nativeWindowHandle.I
// Created by:  drose (30Sep09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: NativeWindowHandle::IntHandle::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NativeWindowHandle::IntHandle::
IntHandle(size_t handle) : _handle(handle) {
}

////////////////////////////////////////////////////////////////////
//     Function: NativeWindowHandle::IntHandle::get_handle
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE size_t NativeWindowHandle::IntHandle::
get_handle() const {
  return _handle;
}

////////////////////////////////////////////////////////////////////
//     Function: NativeWindowHandle::SubprocessHandle::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NativeWindowHandle::SubprocessHandle::
SubprocessHandle(const Filename &filename) : _filename(filename) {
}

////////////////////////////////////////////////////////////////////
//     Function: NativeWindowHandle::SubprocessHandle::get_filename
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE const Filename &NativeWindowHandle::SubprocessHandle::
get_filename() const {
  return _filename;
}

#if defined(HAVE_X11) && !defined(CPPPARSER)
////////////////////////////////////////////////////////////////////
//     Function: NativeWindowHandle::X11Handle::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NativeWindowHandle::X11Handle::
X11Handle(X11_Window handle) : _handle(handle) {
}
#endif  // HAVE_X11

#if defined(HAVE_X11) && !defined(CPPPARSER)
////////////////////////////////////////////////////////////////////
//     Function: NativeWindowHandle::X11Handle::get_handle
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE X11_Window NativeWindowHandle::X11Handle::
get_handle() const {
  return _handle;
}
#endif  // HAVE_X11

#if defined(WIN32) && !defined(CPPPARSER)
////////////////////////////////////////////////////////////////////
//     Function: NativeWindowHandle::WinHandle::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NativeWindowHandle::WinHandle::
WinHandle(HWND handle) : _handle(handle) {
}
#endif  // WIN32

#if defined(WIN32) && !defined(CPPPARSER)
////////////////////////////////////////////////////////////////////
//     Function: NativeWindowHandle::WinHandle::get_handle
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE HWND NativeWindowHandle::WinHandle::
get_handle() const {
  return _handle;
}
#endif  // WIN32
