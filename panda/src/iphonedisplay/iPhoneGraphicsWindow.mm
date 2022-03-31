/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file iPhoneGraphicsWindow.mm
 * @author drose
 * @date 2009-04-08
 */

// We include these system header files first, because there is a namescope
// conflict between them and some other header file that gets included later
// (in particular, TCP_NODELAY must not be a #define symbol for these headers
// to be included properly).

#include <UIKit/UIKit.h>

#include "iPhoneGraphicsWindow.h"
#include "dcast.h"
#include "config_iphonedisplay.h"
#include "iPhoneGraphicsPipe.h"
#include "pStatTimer.h"
#include "glesgsg.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "iPhoneGraphicsStateGuardian.h"
#include "iPhoneGraphicsPipe.h"
#include "throw_event.h"
#include "pnmImage.h"
#include "virtualFileSystem.h"
#include "config_putil.h"
#include "pset.h"
#include "pmutex.h"

TypeHandle IPhoneGraphicsWindow::_type_handle;

/**
 *
 */
IPhoneGraphicsWindow::
IPhoneGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                     const string &name,
                     const FrameBufferProperties &fb_prop,
                     const WindowProperties &win_prop,
                     int flags,
                     GraphicsStateGuardian *gsg,
                     GraphicsOutput *host) :
  GraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  IPhoneGraphicsPipe *ipipe;
  DCAST_INTO_V(ipipe, _pipe);
  ipipe->_graphics_windows.insert(this);
  _gl_view = nil;
  _last_buttons = 0;

  GraphicsWindowInputDevice device =
    GraphicsWindowInputDevice::pointer_and_keyboard(this, "keyboard/mouse");
  _input_devices.push_back(device);
}

/**
 *
 */
IPhoneGraphicsWindow::
~IPhoneGraphicsWindow() {
  if (_gl_view != nil) {
    [ _gl_view release ];
  }
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool IPhoneGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  PStatTimer timer(_make_current_pcollector);

  begin_frame_spam(mode);
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    // not powered up .. just abort..
   return false;
  }

  _gsg->reset_if_new();
  _gsg->set_current_properties(&get_fb_properties());

  return _gsg->begin_frame(current_thread);
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void IPhoneGraphicsWindow::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);

  if (mode == FM_render) {
    nassertv(_gsg != (GraphicsStateGuardian *)NULL);

    copy_to_textures();

    _gsg->end_frame(current_thread);

    if (_gl_view != nil) {
      [_gl_view presentView];
    }
  }
}

/**
 * This function will be called within the draw thread after end_frame() has
 * been called on all windows, to initiate the exchange of the front and back
 * buffers.
 *
 * This should instruct the window to prepare for the flip at the next video
 * sync, but it should not wait.
 *
 * We have the two separate functions, begin_flip() and end_flip(), to make it
 * easier to flip all of the windows at the same time.
 */
void IPhoneGraphicsWindow::
end_flip() {
}

void IPhoneGraphicsWindow::
begin_flip() {
}

/**
 * Required event upcall, used to dispatch window and application events back
 * into panda.
 */
void IPhoneGraphicsWindow::
process_events() {
  GraphicsWindow::process_events();
}

/**
 * Applies the requested set of properties to the window, if possible, for
 * instance to request a change in size or minimization status.
 *
 * The window properties are applied immediately, rather than waiting until
 * the next frame.  This implies that this method may *only* be called from
 * within the window thread.
 *
 * The properties that have been applied are cleared from the structure by
 * this function; so on return, whatever remains in the properties structure
 * are those that were unchanged for some reason (probably because the
 * underlying interface does not support changing that property on an open
 * window).
 */
void IPhoneGraphicsWindow::
set_properties_now(WindowProperties &properties) {
  if (iphonedisplay_cat.is_debug()) {
    iphonedisplay_cat.debug()
      << "------------------------------------------------------\n";
    iphonedisplay_cat.debug()
      << "set_properties_now " << properties << "\n";
  }

  GraphicsWindow::set_properties_now(properties);
}

/**
 * Sets the window's _pipe pointer to NULL; this is generally called only as a
 * precursor to deleting the window.
 */
void IPhoneGraphicsWindow::
clear_pipe() {
  IPhoneGraphicsPipe *ipipe;
  DCAST_INTO_V(ipipe, _pipe);
  ipipe->_graphics_windows.erase(this);

  GraphicsWindow::clear_pipe();
}

/**
 * Called in response to an orientation change event, this tells the window to
 * resize itself according to the new orientation.
 */
void IPhoneGraphicsWindow::
rotate_window() {
  CGRect bounds = [_gl_view bounds];

  WindowProperties properties;
  properties.set_size(bounds.size.width, bounds.size.height);
  system_changed_properties(properties);
}

/**
 * Beginning a single- or multi-touch gesture.
 */
void IPhoneGraphicsWindow::
touches_began(NSSet *touches, UIEvent *event) {
  // Average the position of all of the touches.
  CGPoint location = get_average_location(touches);

  set_pointer_in_window(location.x, location.y);
  handle_button_delta([touches count]);
}

/**
 * Continuing a single- or multi-touch gesture.
 */
void IPhoneGraphicsWindow::
touches_moved(NSSet *touches, UIEvent *event) {
  // Average the position of all of the touches.
  CGPoint location = get_average_location(touches);

  set_pointer_in_window(location.x, location.y);
  handle_button_delta([touches count]);
}

/**
 * Finishing a single- or multi-touch gesture.
 */
void IPhoneGraphicsWindow::
touches_ended(NSSet *touches, UIEvent *event) {
  set_pointer_out_of_window();
  handle_button_delta(0);
}

/**
 * Cancelling a single- or multi-touch gesture.
 */
void IPhoneGraphicsWindow::
touches_cancelled(NSSet *touches, UIEvent *event) {
  set_pointer_out_of_window();
  handle_button_delta(0);
}

/**
 * Returns the average location of all of the indicated touches.
 */
CGPoint IPhoneGraphicsWindow::
get_average_location(NSSet *touches) {
  NSEnumerator *enumerator = [ touches objectEnumerator ];
  CGPoint sum;
  sum.x = 0.0;
  sum.y = 0.0;

  UITouch *touch;
  while ((touch = [ enumerator nextObject ])) {
    CGPoint location = [ touch locationInView: _gl_view ];
    sum.x += location.x;
    sum.y += location.y;
  }

  int count = [touches count];
  sum.x /= count;
  sum.y /= count;
  return sum;
}

/**
 * Closes the window right now.  Called from the window thread.
 */
void IPhoneGraphicsWindow::
close_window() {
  // system_close_window();

  WindowProperties properties;
  properties.set_open(false);
  system_changed_properties(properties);

// release_system_resources(false);
  _gsg.clear();
  _active = false;
  GraphicsWindow::close_window();
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool IPhoneGraphicsWindow::
open_window() {
  nassertr(_gsg == (GraphicsStateGuardian *)NULL, false);

  _gl_view = [ [ EAGLView alloc ] initWithFrame:
        [ [ UIScreen mainScreen ] applicationFrame ]
    ];
  _gl_view->_window = this;

  IPhoneGraphicsPipe *iphonepipe = DCAST(IPhoneGraphicsPipe, _pipe);
  nassertr(iphonepipe != NULL, false);

  iphonepipe->_view_controller.view = _gl_view;
  [ _gl_view layoutSubviews ];

  _gsg = new IPhoneGraphicsStateGuardian(_engine, _pipe, NULL);

  CGRect bounds = [_gl_view bounds];

  _properties.set_size(bounds.size.width, bounds.size.height);
  _properties.set_foreground(true);
  _properties.set_minimized(false);
  _properties.set_open(true);
  _is_valid = true;

  return true;
}

/**
 * Indicates the mouse pointer is seen within the window.
 */
void IPhoneGraphicsWindow::
set_pointer_in_window(int x, int y) {
  _input_devices[0].set_pointer_in_window(x, y);
}

/**
 * Indicates the mouse pointer is no longer within the window.
 */
void IPhoneGraphicsWindow::
set_pointer_out_of_window() {
  _input_devices[0].set_pointer_out_of_window();
}

/**
 * Used to emulate button events
 */
void IPhoneGraphicsWindow::
handle_button_delta(int num_touches) {
  // For now, we'll just map the number of touches to the mouse button number.
  // 1 touch is button 1, 2 touches is button 3 (because this is the normal
  // secondary button), and 3 touches is button 2.

  // This is just a cheesy remapping that will assist migrating applications
  // from standard PC's to iPhone.  It also works well enough within the
  // existing Panda mouse-input framework.  We should expose the full
  // multitouch functionality eventually, but really, the whole mouse-input
  // framework needs a bit of a redesign.

  int new_buttons;
  switch (num_touches) {
  case 0:
    new_buttons = 0x00;
    break;
  case 1:
    new_buttons = 0x01;
    break;
  case 2:
    new_buttons = 0x04;
    break;
  case 3:
  default:
    new_buttons = 0x02;
    break;
  }

  int changed = _last_buttons ^ new_buttons;

  if (changed & 0x01) {
    if (new_buttons & 0x01) {
      _input_devices[0].button_down(MouseButton::one());
    } else {
      _input_devices[0].button_up(MouseButton::one());
    }
  }

  if (changed & 0x04) {
    if (new_buttons & 0x04) {
      _input_devices[0].button_down(MouseButton::two());
    } else {
      _input_devices[0].button_up(MouseButton::two());
    }
  }

  if (changed & 0x02) {
    if (new_buttons & 0x02) {
      _input_devices[0].button_down(MouseButton::three());
    } else {
      _input_devices[0].button_up(MouseButton::three());
    }
  }

  _last_buttons = new_buttons;
}
