// Filename: windowHandle.cxx
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

#include "windowHandle.h"

TypeHandle WindowHandle::_type_handle;
TypeHandle WindowHandle::OSHandle::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: WindowHandle::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
WindowHandle::
~WindowHandle() {
}

////////////////////////////////////////////////////////////////////
//     Function: WindowHandle::get_int_handle
//       Access: Published
//  Description: Returns the OS-specific handle converted to an
//               integer, if this is possible for the particular
//               representation.  Returns 0 if it is not.
////////////////////////////////////////////////////////////////////
size_t WindowHandle::
get_int_handle() const {
  if (_os_handle != NULL) {
    return _os_handle->get_int_handle();
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowHandle::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void WindowHandle::
output(ostream &out) const {
  if (_os_handle == NULL) {
    out << "(null)";
  } else {
    out << *_os_handle;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WindowHandle::attach_child
//       Access: Protected, Virtual
//  Description: Called on a parent handle to indicate a child
//               window's intention to attach itself.
////////////////////////////////////////////////////////////////////
void WindowHandle::
attach_child(WindowHandle *child) {
}

////////////////////////////////////////////////////////////////////
//     Function: WindowHandle::detach_child
//       Access: Protected, Virtual
//  Description: Called on a parent handle to indicate a child
//               window's intention to detach itself.
////////////////////////////////////////////////////////////////////
void WindowHandle::
detach_child(WindowHandle *child) {
}

////////////////////////////////////////////////////////////////////
//     Function: WindowHandle::set_keyboard_focus
//       Access: Protected, Virtual
//  Description: Called on a parent handle to indicate a child
//               window's intention to set itself as the recipient of
//               keyboard events.
////////////////////////////////////////////////////////////////////
void WindowHandle::
set_keyboard_focus(WindowHandle *child) {
}


////////////////////////////////////////////////////////////////////
//     Function: WindowHandle::OSHandle::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
WindowHandle::OSHandle::
~OSHandle() {
}

////////////////////////////////////////////////////////////////////
//     Function: WindowHandle::OSHandle::get_int_handle
//       Access: Published, Virtual
//  Description: Returns the OS-specific handle converted to an
//               integer, if this is possible for the particular
//               representation.  Returns 0 if it is not.
////////////////////////////////////////////////////////////////////
size_t WindowHandle::OSHandle::
get_int_handle() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowHandle::OSHandle::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void WindowHandle::OSHandle::
output(ostream &out) const {
  out << "(no type)";
}
