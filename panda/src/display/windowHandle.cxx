/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file windowHandle.cxx
 * @author drose
 * @date 2009-09-30
 */

#include "windowHandle.h"

TypeHandle WindowHandle::_type_handle;
TypeHandle WindowHandle::OSHandle::_type_handle;

/**
 *
 */
WindowHandle::
~WindowHandle() {
}

/**
 * Call this method on a parent WindowHandle to deliver a Windows message to
 * the current child window, if any.  This is used in the web plugin system to
 * deliver button events detected directly by the browser system into Panda,
 * which is particularly necessary on Vista.
 */
void WindowHandle::
send_windows_message(unsigned int msg, int wparam, int lparam) {
  if (_keyboard_window != nullptr) {
    _keyboard_window->receive_windows_message(msg, wparam, lparam);
  }
}

/**
 * Returns the OS-specific handle converted to an integer, if this is possible
 * for the particular representation.  Returns 0 if it is not.
 */
size_t WindowHandle::
get_int_handle() const {
  if (_os_handle != nullptr) {
    return _os_handle->get_int_handle();
  }
  return 0;
}

/**
 *
 */
void WindowHandle::
output(std::ostream &out) const {
  if (_os_handle == nullptr) {
    out << "(null)";
  } else {
    out << *_os_handle;
  }
}

/**
 * Called on a parent handle to indicate a child window's intention to attach
 * itself.
 */
void WindowHandle::
attach_child(WindowHandle *child) {
}

/**
 * Called on a parent handle to indicate a child window's intention to detach
 * itself.
 */
void WindowHandle::
detach_child(WindowHandle *child) {
  if (_keyboard_window == child) {
    _keyboard_window = nullptr;
  }
}

/**
 * Called on a parent handle to indicate a child window's wish to receive
 * keyboard button events.
 */
void WindowHandle::
request_keyboard_focus(WindowHandle *child) {
  _keyboard_window = child;
}

/**
 * Called on a child handle to deliver a keyboard button event generated in
 * the parent window.
 */
void WindowHandle::
receive_windows_message(unsigned int msg, int wparam, int lparam) {
  nout << "receive_windows_message(" << msg << ", " << wparam << ", " << lparam << ")\n";
}

/**
 *
 */
WindowHandle::OSHandle::
~OSHandle() {
}

/**
 * Returns the OS-specific handle converted to an integer, if this is possible
 * for the particular representation.  Returns 0 if it is not.
 */
size_t WindowHandle::OSHandle::
get_int_handle() const {
  return 0;
}

/**
 *
 */
void WindowHandle::OSHandle::
output(std::ostream &out) const {
  out << "(no type)";
}
