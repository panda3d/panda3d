// Filename: graphicsWindow.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "graphicsWindow.h"
#include "graphicsPipe.h"
#include "config_display.h"
#include "mouseButton.h"
#include "keyboardButton.h"
#include "mutexHolder.h"
#include "hardwareChannel.h"
#include "throw_event.h"

TypeHandle GraphicsWindow::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Constructor
//       Access: Protected
//  Description: Normally, the GraphicsWindow constructor is not
//               called directly; these are created instead via the
//               GraphicsEngine::make_window() function.
////////////////////////////////////////////////////////////////////
GraphicsWindow::
GraphicsWindow(GraphicsPipe *pipe, GraphicsStateGuardian *gsg) :
  GraphicsOutput(pipe, gsg)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif

  if (display_cat.is_debug()) {
    display_cat.debug()
      << "Creating new window using GSG " << (void *)gsg << "\n";
  }

  // Some default properties for windows unless specified otherwise.
  // Other properties (size, title, etc.) must be explicitly
  // specified.
  _properties.set_open(false);
  _properties.set_undecorated(false);
  _properties.set_fullscreen(false);
  _properties.set_minimized(false);
  _properties.set_cursor_hidden(false);

  _window_event = "window-event";
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsWindow::
~GraphicsWindow() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_properties
//       Access: Published
//  Description: Returns the current properties of the window.
////////////////////////////////////////////////////////////////////
WindowProperties GraphicsWindow::
get_properties() const {
  WindowProperties result;
  {
    MutexHolder holder(_properties_lock);
    result = _properties;
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_requested_properties
//       Access: Published
//  Description: Returns the properties of the window that are
//               currently requested.  These properties will be
//               applied to the window (if valid) at the next
//               execution of process_events().
////////////////////////////////////////////////////////////////////
WindowProperties GraphicsWindow::
get_requested_properties() const {
  WindowProperties result;
  {
    MutexHolder holder(_properties_lock);
    result = _requested_properties;
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::clear_rejected_properties
//       Access: Published
//  Description: Empties the set of failed properties that will be
//               returned by get_rejected_properties().
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
clear_rejected_properties() {
  MutexHolder holder(_properties_lock);
  _rejected_properties.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_rejected_properties
//       Access: Published
//  Description: Returns the set of properties that have recently been
//               requested, but could not be applied to the window for
//               some reason.  This set of properties will remain
//               unchanged until they are changed by a new failed
//               request, or clear_rejected_properties() is called.
////////////////////////////////////////////////////////////////////
WindowProperties GraphicsWindow::
get_rejected_properties() const {
  WindowProperties result;
  {
    MutexHolder holder(_properties_lock);
    result = _rejected_properties;
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::request_properties
//       Access: Published
//  Description: Requests a property change on the window.  For
//               example, use this method to request a window change
//               size or minimize or something.
//
//               The change is not made immediately; rather, the
//               request is saved and will be applied the next time
//               the window task is run (probably at the next frame).
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
request_properties(const WindowProperties &requested_properties) {
  MutexHolder holder(_properties_lock);
  _requested_properties.add_properties(requested_properties);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::is_active
//       Access: Published, Virtual
//  Description: Returns true if the window is ready to be rendered
//               into, false otherwise.
////////////////////////////////////////////////////////////////////
bool GraphicsWindow::
is_active() const {
  // Make this smarter?
  return _properties.get_open() && !_properties.get_minimized();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::set_window_event
//       Access: Published
//  Description: Changes the name of the event that is generated when
//               this window is modified externally, e.g. to be
//               resized or closed by the user.
//
//               By default, all windows have the same window event
//               unless they are explicitly changed.  When the event
//               is generated, it includes one parameter: the window
//               itself.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
set_window_event(const string &window_event) {
  MutexHolder holder(_properties_lock);
  _window_event = window_event;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_window_event
//       Access: Published
//  Description: Returns the name of the event that is generated when
//               this window is modified externally, e.g. to be
//               resized or closed by the user.  See set_window_event().
////////////////////////////////////////////////////////////////////
string GraphicsWindow::
get_window_event() const {
  string result;
  MutexHolder holder(_properties_lock);
  result = _window_event;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_num_input_devices
//       Access: Published
//  Description: Returns the number of separate input devices
//               associated with the window.  Typically, a window will
//               have exactly one input device: the keyboard/mouse
//               pair.  However, some windows may have no input
//               devices, and others may add additional devices, for
//               instance for a joystick.
////////////////////////////////////////////////////////////////////
int GraphicsWindow::
get_num_input_devices() const {
  int result;
  {
    MutexHolder holder(_input_lock);
    result = _input_devices.size();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_input_device_name
//       Access: Published
//  Description: Returns the name of the nth input device.
////////////////////////////////////////////////////////////////////
string GraphicsWindow::
get_input_device_name(int device) const {
  string result;
  {
    MutexHolder holder(_input_lock);
    nassertr(device >= 0 && device < (int)_input_devices.size(), "");
    result = _input_devices[device].get_name();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::has_pointer
//       Access: Published
//  Description: Returns true if the nth input device has a
//               screen-space pointer (for instance, a mouse), false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool GraphicsWindow::
has_pointer(int device) const {
  bool result;
  {
    MutexHolder holder(_input_lock);
    nassertr(device >= 0 && device < (int)_input_devices.size(), false);
    result = _input_devices[device].has_pointer();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::has_keyboard
//       Access: Published
//  Description: Returns true if the nth input device has a keyboard,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool GraphicsWindow::
has_keyboard(int device) const {
  bool result;
  {
    MutexHolder holder(_input_lock);
    nassertr(device >= 0 && device < (int)_input_devices.size(), false);
    result = _input_devices[device].has_keyboard();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_mouse_data
//       Access: Public
//  Description: Returns the MouseData associated with the nth input
//               device.
////////////////////////////////////////////////////////////////////
MouseData GraphicsWindow::
get_mouse_data(int device) const {
  MouseData result;
  {
    MutexHolder holder(_input_lock);
    nassertr(device >= 0 && device < (int)_input_devices.size(), MouseData());
    result = _input_devices[device].get_mouse_data();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::has_button_event
//       Access: Public
//  Description: Returns true if the indicated device has a pending
//               button event (a mouse button or keyboard button
//               down/up), false otherwise.  If this returns true, the
//               particular event may be extracted via
//               get_button_event().
////////////////////////////////////////////////////////////////////
bool GraphicsWindow::
has_button_event(int device) const {
  bool result;
  {
    MutexHolder holder(_input_lock);
    nassertr(device >= 0 && device < (int)_input_devices.size(), false);
    result = _input_devices[device].has_button_event();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_button_event
//       Access: Public
//  Description: Assuming a previous call to has_button_event()
//               returned true, this returns the pending button event.
////////////////////////////////////////////////////////////////////
ButtonEvent GraphicsWindow::
get_button_event(int device) {
  ButtonEvent result;
  {
    MutexHolder holder(_input_lock);
    nassertr(device >= 0 && device < (int)_input_devices.size(), ButtonEvent());
    nassertr(_input_devices[device].has_button_event(), ButtonEvent());
    result = _input_devices[device].get_button_event();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::verify_window_sizes
//       Access: Public, Virtual
//  Description: Determines which of the indicated window sizes are
//               supported by available hardware (e.g. in fullscreen
//               mode).
//
//               On entry, dimen is an array containing contiguous x,y
//               pairs specifying possible display sizes; it is
//               numsizes*2 words long.  The function will zero out
//               any invalid x,y size pairs.  The return value is the
//               number of valid sizes that were found.
//
//               Note this doesn't guarantee a resize attempt will
//               work; you still need to check the return value.
//
//               (It might be better to implement some sort of query
//               interface that returns an array of supported sizes,
//               but this way is somewhat simpler and will do the job
//               on most cards, assuming they handle the std sizes the
//               app knows about.)
////////////////////////////////////////////////////////////////////
int GraphicsWindow::
verify_window_sizes(int numsizes, int *dimen) {
  return numsizes;
}
 
////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::request_open
//       Access: Public, Virtual
//  Description: This is called by the GraphicsEngine to request that
//               the window (or whatever) open itself or, in general,
//               make itself valid, at the next call to
//               process_events().
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
request_open() {
  WindowProperties open_properties;
  open_properties.set_open(true);
  request_properties(open_properties);
}
 
////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::request_close
//       Access: Public, Virtual
//  Description: This is called by the GraphicsEngine to request that
//               the window (or whatever) close itself or, in general,
//               make itself invalid, at the next call to
//               process_events().  By that time we promise the gsg
//               pointer will be cleared.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
request_close() {
  WindowProperties close_properties;
  close_properties.set_open(false);
  request_properties(close_properties);
}
 
////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::set_close_now
//       Access: Public, Virtual
//  Description: This is called by the GraphicsEngine to insist that
//               the window be closed immediately.  This is only
//               called from the window thread.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
set_close_now() {
  WindowProperties close_properties;
  close_properties.set_open(false);
  set_properties_now(close_properties);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::process_events
//       Access: Public, Virtual
//  Description: Do whatever processing is necessary to ensure that
//               the window responds to user events.  Also, honor any
//               requests recently made via request_properties().
//
//               This function is called only within the window
//               thread.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
process_events() {
  if (_requested_properties.is_any_specified()) {
    // We don't bother to grab the mutex until after we have already
    // checked whether any properties have been specified.  This is
    // technically sloppy, but it ought to be o.k. since it's just a
    // bitmask after all.
    WindowProperties properties;
    {
      MutexHolder holder(_properties_lock);
      properties = _requested_properties;
      _requested_properties.clear();

      set_properties_now(properties);
      if (properties.is_any_specified()) {
        display_cat.info()
          << "Unable to set window properties: " << properties << "\n";
        _rejected_properties.add_properties(properties);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::set_properties_now
//       Access: Public, Virtual
//  Description: Applies the requested set of properties to the
//               window, if possible, for instance to request a change
//               in size or minimization status.
//
//               The window properties are applied immediately, rather
//               than waiting until the next frame.  This implies that
//               this method may *only* be called from within the
//               window thread.
//
//               The properties that have been applied are cleared
//               from the structure by this function; so on return,
//               whatever remains in the properties structure are
//               those that were unchanged for some reason (probably
//               because the underlying interface does not support
//               changing that property on an open window).
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
set_properties_now(WindowProperties &properties) {
  if (properties.has_open() && 
      properties.get_open() != _properties.get_open()) {
    // Open or close a new window.  In this case we can get all of the
    // properties at once.
    _properties.add_properties(properties);
    properties.clear();

    if (_properties.get_open()) {
      if (open_window()) {
        // When the window is first opened, force its size to be
        // broadcast to its display regions.
        _x_size = _properties.get_x_size();
        _y_size = _properties.get_y_size();
        _has_size = true;
        _is_valid = true;

        Channels::iterator ci;
        for (ci = _channels.begin(); ci != _channels.end(); ++ci) {
          GraphicsChannel *chan = (*ci);
          chan->window_resized(_x_size, _y_size);
        }

      } else {
        // Since we can't even open the window, tag the
        // _rejected_properties with all of the window properties that
        // failed.
        _rejected_properties.add_properties(_properties);

        // And mark the window closed.
        _properties.set_open(false);
      }

    } else {
      // We can't close the window if its GSG hasn't been released
      // yet.
      nassertv(_gsg == (GraphicsStateGuardian *)NULL);
      close_window();
      _is_valid = false;
    }
    return;
  }

  if (!_properties.get_open()) {
    // The window is not currently open; we can set properties at
    // will.
    _properties.add_properties(properties);
    properties.clear();
    return;
  }

  // The window is already open; we are limited to what we can change
  // on the fly.

  if (properties.has_size() || properties.has_origin()) {
    // Consider changing the window's size and/or position.
    WindowProperties reshape_props;
    if (properties.has_size()) {
      reshape_props.set_size(properties.get_x_size(), properties.get_y_size());
    } else {
      reshape_props.set_size(_properties.get_x_size(), _properties.get_y_size());
    }
    if (properties.has_origin() && !is_fullscreen()) {
      reshape_props.set_origin(properties.get_x_origin(), properties.get_y_origin());
    } else {
      reshape_props.set_origin(_properties.get_x_origin(), _properties.get_y_origin());
    }
    
    if (reshape_props.get_x_size() != _properties.get_x_size() ||
        reshape_props.get_y_size() != _properties.get_y_size() ||
        reshape_props.get_x_origin() != _properties.get_x_origin() ||
        reshape_props.get_y_origin() != _properties.get_y_origin()) {
      if (do_reshape_request(reshape_props.get_x_origin(),
                             reshape_props.get_y_origin(),
                             reshape_props.get_x_size(),
                             reshape_props.get_y_size())) {
        system_changed_size(reshape_props.get_x_size(), 
                            reshape_props.get_y_size());
        _properties.add_properties(reshape_props);
        properties.clear_size();
        properties.clear_origin();
      }
    }
  }

  if (properties.has_fullscreen() && 
      properties.get_fullscreen() == _properties.get_fullscreen()) {
    // Fullscreen property specified, but unchanged.
    properties.clear_fullscreen();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::close_window
//       Access: Protected, Virtual
//  Description: Closes the window right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
close_window() {
  display_cat.info()
    << "Closing " << get_type() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::open_window
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool GraphicsWindow::
open_window() {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::reset_window
//       Access: Protected, Virtual
//  Description: resets the window framebuffer from its derived
//               children. Does nothing here.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
reset_window(bool swapchain) {
  display_cat.info()
    << "Resetting " << get_type() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::do_reshape_request
//       Access: Protected, Virtual
//  Description: Called from the window thread in response to a request
//               from within the code (via request_properties()) to
//               change the size and/or position of the window.
//               Returns true if the window is successfully changed,
//               or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool GraphicsWindow::
do_reshape_request(int x_origin, int y_origin, int x_size, int y_size) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::system_changed_properties
//       Access: Protected
//  Description: Should be called (from within the window thread) when
//               process_events() detects an external change in some
//               important window property; for instance, when the
//               user resizes the window.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
system_changed_properties(const WindowProperties &properties) {
  MutexHolder holder(_properties_lock);

  if (properties.has_size()) {
    system_changed_size(properties.get_x_size(), properties.get_y_size());
  }

  WindowProperties old_properties = _properties;
  _properties.add_properties(properties);
  if (_properties != old_properties) {
    throw_event(_window_event, this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::system_changed_size
//       Access: Protected
//  Description: An internal function to update all the channels with
//               the new size of the window.  This should always be
//               called before changing the _size members of the
//               _properties structure.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
system_changed_size(int x_size, int y_size) {
  if (x_size != _properties.get_x_size() || 
      y_size != _properties.get_y_size()) {
    _x_size = x_size;
    _y_size = y_size;
    _has_size = true;
    
    Channels::iterator ci;
    for (ci = _channels.begin(); ci != _channels.end(); ++ci) {
      GraphicsChannel *chan = (*ci);
      chan->window_resized(x_size, y_size);
    }
  }
}
