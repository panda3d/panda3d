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

#include "pmap.h"

TypeHandle GraphicsWindow::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Constructor
//       Access: Protected
//  Description: Normally, the GraphicsWindow constructor is not
//               called directly; these are created instead via the
//               GraphicsEngine::make_window() function.
////////////////////////////////////////////////////////////////////
GraphicsWindow::
GraphicsWindow(GraphicsPipe *pipe) {
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
  _pipe = pipe;

  // Some default properties for windows unless specified otherwise.
  // Other properties (size, title, etc.) must be explicitly
  // specified.
  _properties.set_open(false);
  _properties.set_undecorated(false);
  _properties.set_fullscreen(false);
  _properties.set_minimized(false);
  _properties.set_cursor_hidden(false);
  _properties.set_depth_bits(1);
  _properties.set_color_bits(1);
  _properties.set_framebuffer_mode(WindowProperties::FM_rgba | 
                                   WindowProperties::FM_double_buffer | 
                                   WindowProperties::FM_depth);
  _display_regions_stale = false;
  _window_event = "window-event";

  // By default, windows are set up to clear color and depth.
  set_clear_color_active(true);
  set_clear_depth_active(true);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Copy Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsWindow::
GraphicsWindow(const GraphicsWindow &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Copy Assignment Operator
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
operator = (const GraphicsWindow &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsWindow::
~GraphicsWindow() {
  // The window should be closed by the time we destruct.
  nassertv(!_properties.get_open());

  // And we shouldn't have a GraphicsPipe pointer anymore.
  nassertv(_pipe == (GraphicsPipe *)NULL);

  // We don't have to destruct our child channels explicitly, since
  // they are all reference-counted and will go away when their
  // pointers do.  However, we do need to zero out their pointers to
  // us.
  Channels::iterator ci;
  for (ci = _channels.begin(); ci != _channels.end(); ++ci) {
    (*ci)->_window = NULL;
  }
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
    MutexHolder holder(_lock);
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
    MutexHolder holder(_lock);
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
  MutexHolder holder(_lock);
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
    MutexHolder holder(_lock);
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
  MutexHolder holder(_lock);
  _requested_properties.add_properties(requested_properties);
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
  MutexHolder holder(_lock);
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
  MutexHolder holder(_lock);
  result = _window_event;
  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_channel
//       Access: Public
//  Description: Returns a GraphicsChannel pointer that can be used to
//               access the indicated channel number.  All windows
//               have at least one channel, channel 0, which
//               corresponds to the entire window.  If the hardware
//               supports it, some kinds of windows may also have a
//               number of hardware channels available at indices
//               1..n, which will correspond to a subregion of the
//               window.
//
//               This function returns a GraphicsChannel pointer if a
//               channel is available, or NULL if it is not.  If
//               called twice with the same index number, it will
//               return the same pointer.
////////////////////////////////////////////////////////////////////
GraphicsChannel *GraphicsWindow::
get_channel(int index) {
  MutexHolder holder(_lock);
  nassertr(index >= 0, NULL);

  if (index < (int)_channels.size()) {
    if (_channels[index] != (GraphicsChannel *)NULL) {
      return _channels[index];
    }
  }

  // This channel has never been requested before; define it.

  PT(GraphicsChannel) chan;
  if (index == 0) {
    // Channel 0 is the default channel: the entire screen.
    chan = new GraphicsChannel(this);
  } else {
    // Any other channel is some hardware-specific channel.
    GraphicsPipe *pipe = _pipe;
    if (pipe != (GraphicsPipe *)NULL) {
      chan = _pipe->get_hw_channel(this, index);
      if (chan == (GraphicsChannel *)NULL) {
        display_cat.error()
          << "GraphicsWindow::get_channel() - got a NULL channel" << endl;
      } else {
        if (chan->get_window() != this) {
          chan = NULL;
        }
      }
    }
  }

  if (chan != (GraphicsChannel *)NULL) {
    declare_channel(index, chan);
  }

  return chan;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::remove_channel
//       Access: Public
//  Description: Deletes a GraphicsChannel that was previously created
//               via a call to get_channel().  Note that the channel
//               is not actually deleted until all pointers to it are
//               cleared.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
remove_channel(int index) {
  MutexHolder holder(_lock);
  if (index >= 0 && index < (int)_channels.size()) {
    _channels[index].clear();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_max_channel_index
//       Access: Public
//  Description: Returns the largest channel index number yet created,
//               plus 1.  All channels associated with this window
//               will have an index number in the range [0,
//               get_max_channel_index()).  This function, in
//               conjunction with is_channel_defined(), below, may be
//               used to determine the complete set of channels
//               associated with the window.
////////////////////////////////////////////////////////////////////
int GraphicsWindow::
get_max_channel_index() const {
  int result;
  {
    MutexHolder holder(_lock);
    result = _channels.size();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::is_channel_defined
//       Access: Public
//  Description: Returns true if the channel with the given index
//               number has already been defined, false if it hasn't.
//               If this returns true, calling get_channel() on the
//               given index number will return the channel pointer.
//               If it returns false, calling get_channel() will
//               create and return a new channel pointer.
////////////////////////////////////////////////////////////////////
bool GraphicsWindow::
is_channel_defined(int index) const {
  bool result;
  {
    MutexHolder holder(_lock);
    if (index < 0 || index >= (int)_channels.size()) {
      result = false;
    } else {
      result = (_channels[index] != (GraphicsChannel *)NULL);
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_num_display_regions
//       Access: Published
//  Description: Returns the number of active DisplayRegions that have
//               been created within the various layers and channels
//               of the window.
////////////////////////////////////////////////////////////////////
int GraphicsWindow::
get_num_display_regions() const {
  determine_display_regions();
  int result;
  {
    MutexHolder holder(_lock);
    result = _display_regions.size();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_display_region
//       Access: Published
//  Description: Returns the nth active DisplayRegion of those that
//               have been created within the various layers and
//               channels of the window.  This may return NULL if n is
//               out of bounds; particularly likely if the number of
//               display regions has changed since the last call to
//               get_num_display_regions().
////////////////////////////////////////////////////////////////////
DisplayRegion *GraphicsWindow::
get_display_region(int n) const {
  determine_display_regions();
  DisplayRegion *result;
  {
    MutexHolder holder(_lock);
    if (n >= 0 && n < (int)_display_regions.size()) {
      result = _display_regions[n];
    } else {
      result = NULL;
    }
  }
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
//     Function: GraphicsWindow::make_scratch_display_region
//       Access: Public
//  Description: Allocates and returns a temporary DisplayRegion that
//               may be used to render offscreen into.  This
//               DisplayRegion is not associated with any layer.
//
//               To allocate a normal DisplayRegion for rendering, use
//               the interface provided in GraphicsLayer.
////////////////////////////////////////////////////////////////////
PT(DisplayRegion) GraphicsWindow::
make_scratch_display_region(int x_size, int y_size) const {
#ifndef NDEBUG
  {
    MutexHolder holder(_lock);
    if (x_size > _properties.get_x_size() || 
        y_size > _properties.get_y_size()) {
      display_cat.error()
        << "make_scratch_display_region(): requested region of size " 
        << x_size << ", " << y_size << " is larger than window of size "
        << _properties.get_x_size() << ", " << _properties.get_y_size()
        << ".\n";
      x_size = min(x_size, _properties.get_x_size());
      y_size = min(y_size, _properties.get_y_size());
    }
  }
#endif

  PT(DisplayRegion) region = new DisplayRegion(x_size, y_size);
  region->copy_clear_settings(*this);
  return region;
}
 
////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool GraphicsWindow::
begin_frame() {
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    MutexHolder holder(_lock);
    // Oops, we don't have a GSG yet.
    if (!_properties.get_open()) {
      return false;
    }
    make_gsg();
    if (_gsg == (GraphicsStateGuardian *)NULL) {
      // Still couldn't make the GSG for some reason.
      return false;
    }
  } else {
    // Okay, we already have a GSG, so activate it.
    make_current();
  }

  return _gsg->begin_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::clear
//       Access: Public
//  Description: Clears the entire framebuffer before rendering,
//               according to the settings of get_color_clear_active()
//               and get_depth_clear_active() (inherited from
//               ClearableRegion).
//
//               This function is called only within the draw thread.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
clear() {
  if (is_any_clear_active()) {
    nassertv(_gsg != (GraphicsStateGuardian *)NULL);

    int x_size, y_size;
    {
      MutexHolder holder(_lock);
      x_size = _properties.get_x_size();
      y_size = _properties.get_y_size();
    }
    PT(DisplayRegion) win_dr =
      make_scratch_display_region(x_size, y_size);
    DisplayRegionStack old_dr = _gsg->push_display_region(win_dr);
    _gsg->clear(this);
    _gsg->pop_display_region(old_dr);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
end_frame() {
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);
  _gsg->end_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::make_gsg
//       Access: Public, Virtual
//  Description: Creates a new GSG for the window and stores it in the
//               _gsg pointer.  This should only be called from within
//               the draw thread.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
make_gsg() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::release_gsg
//       Access: Public, Virtual
//  Description: Releases the current GSG pointer, if it is currently
//               held, and resets the GSG to NULL.  This should only
//               be called from within the draw thread.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
release_gsg() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    _gsg->close_gsg();
    _gsg.clear();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::make_current
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               during begin_frame() to ensure the graphics context
//               is ready for drawing.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
make_current() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::begin_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after end_frame() has been called on all windows, to
//               initiate the exchange of the front and back buffers.
//
//               This should instruct the window to prepare for the
//               flip at the next video sync, but it should not wait.
//
//               We have the two separate functions, begin_flip() and
//               end_flip(), to make it easier to flip all of the
//               windows at the same time.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
begin_flip() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::end_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after begin_flip() has been called on all windows, to
//               finish the exchange of the front and back buffers.
//
//               This should cause the window to wait for the flip, if
//               necessary.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
end_flip() {
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
      MutexHolder holder(_lock);
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
        Channels::iterator ci;
        for (ci = _channels.begin(); ci != _channels.end(); ++ci) {
          GraphicsChannel *chan = (*ci);
          chan->window_resized(_properties.get_x_size(), 
                               _properties.get_y_size());
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
//     Function: GraphicsWindow::open_window
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
  MutexHolder holder(_lock);

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
    Channels::iterator ci;
    for (ci = _channels.begin(); ci != _channels.end(); ++ci) {
      GraphicsChannel *chan = (*ci);
      chan->window_resized(x_size, y_size);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::declare_channel
//       Access: Protected
//  Description: An internal function to add the indicated
//               newly-created channel to the list at the indicated
//               channel number.
//
//               The caller must grab and hold _lock before making
//               this call.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
declare_channel(int index, GraphicsChannel *chan) {
  nassertv(index >= 0);
  if (index >= (int)_channels.size()) {
    _channels.reserve(index);
    while (index >= (int)_channels.size()) {
      _channels.push_back(NULL);
    }
  }

  nassertv(index < (int)_channels.size());
  _channels[index] = chan;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::do_determine_display_regions
//       Access: Private
//  Description: Recomputes the list of active DisplayRegions within
//               the window.
////////////////////////////////////////////////////////////////////
void GraphicsWindow::
do_determine_display_regions() {
  MutexHolder holder(_lock);
  _display_regions_stale = false;
  _display_regions.clear();
  Channels::const_iterator ci;
  for (ci = _channels.begin(); ci != _channels.end(); ++ci) {
    GraphicsChannel *chan = (*ci);
    if (chan->is_active()) {
      GraphicsChannel::GraphicsLayers::const_iterator li;
      for (li = chan->_layers.begin(); li != chan->_layers.end(); ++li) {
        GraphicsLayer *layer = (*li);
        if (layer->is_active()) {
          GraphicsLayer::DisplayRegions::const_iterator dri;
          for (dri = layer->_display_regions.begin(); 
               dri != layer->_display_regions.end(); 
               ++dri) {
            DisplayRegion *dr = (*dri);
            if (dr->is_active()) {
              _display_regions.push_back(dr);
            }
          }
        }
      }
    }
  }
}
