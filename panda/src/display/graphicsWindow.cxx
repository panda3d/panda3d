// Filename: graphicsWindow.cxx
// Created by:  mike (09Jan97)
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

#include "graphicsWindow.h"
#include "graphicsPipe.h"
#include "config_display.h"
#include "mouseButton.h"
#include "keyboardButton.h"
#include "lightMutexHolder.h"
#include "lightReMutexHolder.h"
#include "throw_event.h"
#include "string_utils.h"

TypeHandle GraphicsWindow::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Constructor
//       Access: Protected
//  Description: Normally, the GraphicsWindow constructor is not
//               called directly; these are created instead via the
//               GraphicsEngine::make_window() function.
////////////////////////////////////////////////////////////////////
GraphicsWindow::
GraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
               const string &name,
               const FrameBufferProperties &fb_prop,
               const WindowProperties &win_prop,
               int flags,
               GraphicsStateGuardian *gsg,
               GraphicsOutput *host) :
  GraphicsOutput(engine, pipe, name, fb_prop, win_prop, flags, gsg, host, true),
  _input_lock("GraphicsWindow::_input_lock"),
  _properties_lock("GraphicsWindow::_properties_lock")
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif

  if (display_cat.is_debug()) {
    display_cat.debug()
      << "Creating new window " << get_name() << "\n";
  }

  _properties.set_open(false);
  _properties.set_undecorated(false);
  _properties.set_fullscreen(false);
  _properties.set_minimized(false);
  _properties.set_cursor_hidden(false);

  request_properties(WindowProperties::get_default());
  request_properties(win_prop);

  _window_event = "window-event";
  _got_expose_event = false;
  _unexposed_draw = win_unexposed_draw;
  set_pixel_zoom(pixel_zoom);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsWindow::
~GraphicsWindow() {
  // Clean up python event handlers.
#ifdef HAVE_PYTHON
  PythonWinProcClasses::iterator iter;
  for (iter = _python_window_proc_classes.begin();
       iter != _python_window_proc_classes.end();
       ++iter) {
    delete *iter;
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_properties
//       Access: Published
//  Description: Returns the current properties of the window.
////////////////////////////////////////////////////////////////////
const WindowProperties GraphicsWindow::
get_properties() const {
  WindowProperties result;
  {
    LightReMutexHolder holder(_properties_lock);
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
const WindowProperties GraphicsWindow::
get_requested_properties() const {
  WindowProperties result;
  {
    LightReMutexHolder holder(_properties_lock);
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
  LightReMutexHolder holder(_properties_lock);
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
    LightReMutexHolder holder(_properties_lock);
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
  LightReMutexHolder holder(_properties_lock);
  _requested_properties.add_properties(requested_properties);

  if (!_has_size && _requested_properties.has_size()) {
    // If we just requested a particular size, anticipate that it will
    // stick.  This is helpful for the MultitexReducer, which needs to
    // know the size of the textures that it will be working with,
    // even if the texture hasn't been fully generated yet.
    _x_size = _requested_properties.get_x_size();
    _y_size = _requested_properties.get_y_size();

    // Don't set _has_size yet, because we don't really know yet.
  }
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
  return GraphicsOutput::is_active() && _properties.get_open() && !_properties.get_minimized();
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
  LightReMutexHolder holder(_properties_lock);
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
  LightReMutexHolder holder(_properties_lock);
  result = _window_event;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::set_close_request_event
//       Access: Published
//  Description: Sets the event that is triggered when the user
//               requests to close the window, e.g. via alt-F4, or
//               clicking on the close box.
//
//               The default for each window is for this event to be
//               the empty string, which means the window-close
//               request is handled immediately by Panda (and the
//               window will be closed without the app getting a
//               chance to intervene).  If you set this to a nonempty
//               string, then the window is not closed, but instead
//               the event is thrown.  It is then up to the app to
//               respond appropriately, for instance by presenting an
//               "are you sure?" dialog box, and eventually calling
//               close_window() when the user is sure.
//
//               It is considered poor form to set this string and
//               then not handle the event.  This can frustrate the
//               user by making it difficult for him to cleanly shut
//               down the application (and may force the user to
//               hard-kill the app, or reboot the machine).
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
set_close_request_event(const string &close_request_event) {
  LightReMutexHolder holder(_properties_lock);
  _close_request_event = close_request_event;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_close_request_event
//       Access: Published
//  Description: Returns the name of the event set via
//               set_close_request_event().  If this string is
//               nonempty, then when the user requests to close
//               window, this event will be generated instead.  See
//               set_close_request_event().
////////////////////////////////////////////////////////////////////
string GraphicsWindow::
get_close_request_event() const {
  string result;
  LightReMutexHolder holder(_properties_lock);
  result = _close_request_event;
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
    LightMutexHolder holder(_input_lock);
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
    LightMutexHolder holder(_input_lock);
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
    LightMutexHolder holder(_input_lock);
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
    LightMutexHolder holder(_input_lock);
    nassertr(device >= 0 && device < (int)_input_devices.size(), false);
    result = _input_devices[device].has_keyboard();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: x11GraphicsWindow::get_keyboard_map
//       Access: Published, Virtual
//  Description: Returns a ButtonMap containing the association
//               between raw buttons and virtual buttons.
////////////////////////////////////////////////////////////////////
ButtonMap *GraphicsWindow::
get_keyboard_map() const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::enable_pointer_events
//       Access: Published
//  Description: Turn on the generation of pointer events.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
enable_pointer_events(int device) {
  LightMutexHolder holder(_input_lock);
  nassertv(device >= 0 && device < (int)_input_devices.size());
  _input_devices[device].enable_pointer_events();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::disable_pointer_events
//       Access: Published
//  Description: Turn off the generation of pointer events.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
disable_pointer_events(int device) {
  LightMutexHolder holder(_input_lock);
  nassertv(device >= 0 && device < (int)_input_devices.size());
  _input_devices[device].disable_pointer_events();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::enable_pointer_mode
//       Access: Published
//  Description: See GraphicsWindowInputDevice::enable_pointer_mode
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
enable_pointer_mode(int device, double speed) {
  LightMutexHolder holder(_input_lock);
  nassertv(device >= 0 && device < (int)_input_devices.size());
  _input_devices[device].enable_pointer_mode(speed);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::disable_pointer_events
//       Access: Published
//  Description: See GraphicsWindowInputDevice::disable_pointer_mode
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
disable_pointer_mode(int device) {
  LightMutexHolder holder(_input_lock);
  nassertv(device >= 0 && device < (int)_input_devices.size());
  _input_devices[device].disable_pointer_mode();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_pointer
//       Access: Published
//  Description: Returns the MouseData associated with the nth
//               input device's pointer.  This is deprecated; use
//               get_pointer_device().get_pointer() instead, or for
//               raw mice, use the InputDeviceManager interface.
////////////////////////////////////////////////////////////////////
MouseData GraphicsWindow::
get_pointer(int device) const {
  MouseData result;
  {
    LightMutexHolder holder(_input_lock);
    nassertr(device >= 0 && device < (int)_input_devices.size(), MouseData());
    result = _input_devices[device].get_pointer();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::move_pointer
//       Access: Published, Virtual
//  Description: Forces the pointer to the indicated position within
//               the window, if possible.
//
//               Returns true if successful, false on failure.  This
//               may fail if the mouse is not currently within the
//               window, or if the API doesn't support this operation.
////////////////////////////////////////////////////////////////////
bool GraphicsWindow::
move_pointer(int, int, int) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::close_ime
//       Access: Published, Virtual
//  Description: Forces the ime window to close if any
//
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
close_ime() {
  return;
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
    LightMutexHolder holder(_input_lock);
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
    LightMutexHolder holder(_input_lock);
    nassertr(device >= 0 && device < (int)_input_devices.size(), ButtonEvent());
    nassertr(_input_devices[device].has_button_event(), ButtonEvent());
    result = _input_devices[device].get_button_event();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::has_pointer_event
//       Access: Public
//  Description: Returns true if the indicated device has a pending
//               pointer event (a mouse movement).  If this returns
//               true, the particular event may be extracted via
//               get_pointer_events().
////////////////////////////////////////////////////////////////////
bool GraphicsWindow::
has_pointer_event(int device) const {
  bool result;
  {
    LightMutexHolder holder(_input_lock);
    nassertr(device >= 0 && device < (int)_input_devices.size(), false);
    result = _input_devices[device].has_pointer_event();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_pointer_events
//       Access: Public
//  Description: Assuming a previous call to has_pointer_event()
//               returned true, this returns the pending pointer event list.
////////////////////////////////////////////////////////////////////
PT(PointerEventList) GraphicsWindow::
get_pointer_events(int device) {
  PT(PointerEventList) result;
  {
    LightMutexHolder holder(_input_lock);
    nassertr(device >= 0 && device < (int)_input_devices.size(), NULL);
    nassertr(_input_devices[device].has_pointer_event(), NULL);
    result = _input_devices[device].get_pointer_events();
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
      LightReMutexHolder holder(_properties_lock);
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
        _is_valid = true;
        set_size_and_recalc(_properties.get_x_size(),
                            _properties.get_y_size());
      } else {
        // Since we can't even open the window, tag the
        // _rejected_properties with all of the window properties that
        // failed.
        _rejected_properties.add_properties(_properties);

        // And mark the window closed.
        _properties.set_open(false);
        _is_valid = false;
      }

    } else {
      // We used to resist closing a window before its GSG has been
      // released.  Now it seems we never release a GSG, so go ahead
      // and close the window.
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

  properties.clear_open();

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
    } else if (_properties.has_origin()) {
      reshape_props.set_origin(_properties.get_x_origin(), _properties.get_y_origin());
    }

    bool has_origin = reshape_props.has_origin();
    int x_origin = 0, y_origin = 0;
    if (has_origin) {
      x_origin = reshape_props.get_x_origin();
      y_origin = reshape_props.get_y_origin();
    }
    
    if (reshape_props.get_x_size() != _properties.get_x_size() ||
        reshape_props.get_y_size() != _properties.get_y_size() ||
        (has_origin && (x_origin != _properties.get_x_origin() ||
                        y_origin != _properties.get_y_origin()))) {
      if (do_reshape_request(x_origin, y_origin, has_origin,
                             reshape_props.get_x_size(),
                             reshape_props.get_y_size())) {
        properties.clear_size();
        properties.clear_origin();
      }
    } else {
      properties.clear_size();
      properties.clear_origin();
    }
  }

  if (properties.has_fullscreen() && 
      properties.get_fullscreen() == _properties.get_fullscreen()) {
    // Fullscreen property specified, but unchanged.
    properties.clear_fullscreen();
  }
  if (properties.has_mouse_mode() ) {
    
    if (properties.get_mouse_mode() == _properties.get_mouse_mode()) {  
      properties.clear_mouse_mode();
    }
    else {
      if(properties.get_mouse_mode() == WindowProperties::M_absolute) {
        _properties.set_mouse_mode(WindowProperties::M_absolute);
        mouse_mode_absolute();
        properties.clear_mouse_mode();
      }
      else
      {
        _properties.set_mouse_mode(WindowProperties::M_relative);
        mouse_mode_relative();
        properties.clear_mouse_mode();        
      }    
    }
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

  // Tell our parent window (if any) that we're no longer its child.
  if (_window_handle != (WindowHandle *)NULL &&
      _parent_window_handle != (WindowHandle *)NULL) {
    _parent_window_handle->detach_child(_window_handle);
  }

  _window_handle = NULL;
  _parent_window_handle = NULL;
  _is_valid = false;
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
do_reshape_request(int x_origin, int y_origin, bool has_origin,
                   int x_size, int y_size) {
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
  if (display_cat.is_debug()) {
    display_cat.debug()
      << "system_changed_properties(" << properties << ")\n";
  }

  LightReMutexHolder holder(_properties_lock);

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
//  Description: An internal function to update all the DisplayRegions
//               with the new size of the window.  This should always
//               be called before changing the _size members of the
//               _properties structure.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
system_changed_size(int x_size, int y_size) {
  if (display_cat.is_debug()) {
    display_cat.debug()
      << "system_changed_size(" << x_size << ", " << y_size << ")\n";
  }
  
  if (!_properties.has_size() || (x_size != _properties.get_x_size() || 
                                  y_size != _properties.get_y_size())) {
    set_size_and_recalc(x_size, y_size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::add_input_device
//       Access: Protected
//  Description: Adds a GraphicsWindowInputDevice to the vector.
//               Returns the index of the new device.
////////////////////////////////////////////////////////////////////
int GraphicsWindow::
add_input_device(const GraphicsWindowInputDevice &device) {
  LightMutexHolder holder(_input_lock);
  int index = (int)_input_devices.size();
  _input_devices.push_back(device);
  _input_devices.back().set_device_index(index);
  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::mouse_mode_relative
//       Access: Protected, Virtual
//  Description: detaches mouse. Only mouse delta from now on. 
//               
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
mouse_mode_relative() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::mouse_mode_absolute
//       Access: Protected, Virtual
//  Description: reattaches mouse to location
//               
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
mouse_mode_absolute() {

}

#ifdef HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::add_custom_event_handler
//       Access: Published
//  Description: Adds a python event handler to be called
//               when a window event occurs.
//               
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
add_python_event_handler(PyObject* handler, PyObject* name){
  PythonGraphicsWindowProc* pgwp = new PythonGraphicsWindowProc(handler, name);
  _python_window_proc_classes.insert(pgwp);
  add_window_proc(pgwp);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::remove_custom_event_handler
//       Access: Published
//  Description: Removes the specified python event handler.
//               
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
remove_python_event_handler(PyObject* name){
  list<PythonGraphicsWindowProc*> toRemove;
  PythonWinProcClasses::iterator iter;
  for (iter = _python_window_proc_classes.begin(); iter != _python_window_proc_classes.end(); ++iter) {
    PythonGraphicsWindowProc* pgwp = *iter;
    if (PyObject_RichCompareBool(pgwp->get_name(), name, Py_EQ) == 1) {
      toRemove.push_back(pgwp);
    }
#if PY_MAJOR_VERSION < 3
    else if (PyObject_Compare(pgwp->get_name(), name) == 0) {
      toRemove.push_back(pgwp);
    }
#endif
  }
  list<PythonGraphicsWindowProc*>::iterator iter2;
  for (iter2 = toRemove.begin(); iter2 != toRemove.end(); ++iter2) {
    PythonGraphicsWindowProc* pgwp = *iter2;
    remove_window_proc(pgwp);
    _python_window_proc_classes.erase(pgwp);
    delete pgwp;
  }
}

#endif // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::is_touch_event
//       Access: Published, Virtual
//  Description: Returns whether the specified event msg is a touch message.
//               
////////////////////////////////////////////////////////////////////
bool GraphicsWindow::
is_touch_event(GraphicsWindowProcCallbackData* callbackData){
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_num_touches
//       Access: Published, Virtual
//  Description: Returns the current number of touches on this window.
//               
////////////////////////////////////////////////////////////////////
int GraphicsWindow::
get_num_touches(){
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_touch_info
//       Access: Published, Virtual
//  Description: Returns the TouchInfo object describing the specified touch.
//               
////////////////////////////////////////////////////////////////////
TouchInfo GraphicsWindow::
get_touch_info(int index){
  return TouchInfo();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::supports_window_procs
//       Access: Published, Virtual
//  Description: Returns whether this window supports adding of Windows proc handlers.
//               
////////////////////////////////////////////////////////////////////
bool GraphicsWindow::supports_window_procs() const{
  return false;
}
