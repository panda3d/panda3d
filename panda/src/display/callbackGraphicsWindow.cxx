/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file callbackGraphicsWindow.cxx
 * @author drose
 * @date 2011-01-06
 */

#include "callbackGraphicsWindow.h"

TypeHandle CallbackGraphicsWindow::_type_handle;
TypeHandle CallbackGraphicsWindow::WindowCallbackData::_type_handle;
TypeHandle CallbackGraphicsWindow::EventsCallbackData::_type_handle;
TypeHandle CallbackGraphicsWindow::PropertiesCallbackData::_type_handle;
TypeHandle CallbackGraphicsWindow::RenderCallbackData::_type_handle;

/**
 * Use GraphicsEngine::make_output() to construct a CallbackGraphicsWindow.
 */
CallbackGraphicsWindow::
CallbackGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                       const std::string &name,
                       const FrameBufferProperties &fb_prop,
                       const WindowProperties &win_prop,
                       int flags,
                       GraphicsStateGuardian *gsg) :
  GraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, nullptr)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif

  // Let's ensure that these properties are set to *something* initially.
  _properties.set_origin(0, 0);
  _properties.set_size(0, 0);
}

/**
 *
 */
CallbackGraphicsWindow::
~CallbackGraphicsWindow() {
}

/**
 * Adds a new input device (mouse) to the window with the indicated name.
 * Returns the index of the new device.
 */
int CallbackGraphicsWindow::
create_input_device(const std::string &name) {
  return add_input_device(GraphicsWindowInputDevice::pointer_and_keyboard(this, name));
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool CallbackGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  bool result = false;
  if (_render_callback != nullptr) {
    RenderCallbackData data(this, RCT_begin_frame, mode);
    _render_callback->do_callback(&data);
    result = data.get_render_flag();
  } else {
    result = GraphicsWindow::begin_frame(mode, current_thread);
  }

  if (!result) {
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
void CallbackGraphicsWindow::
end_frame(FrameMode mode, Thread *current_thread) {
  if (_render_callback != nullptr) {
    // In case the callback or the application hosting the OpenGL context
    // wants to do more rendering, let's give it a blank slate.
    _gsg->set_state_and_transform(RenderState::make_empty(), _gsg->get_internal_transform());
    _gsg->clear_before_callback();

    RenderCallbackData data(this, RCT_end_frame, mode);
    _render_callback->do_callback(&data);
  } else {
    GraphicsWindow::end_frame(mode, current_thread);
  }

  _gsg->end_frame(current_thread);

  if (mode == FM_render) {
    trigger_flip();
    clear_cube_map_selection();
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
void CallbackGraphicsWindow::
begin_flip() {
  if (_render_callback != nullptr) {
    RenderCallbackData data(this, RCT_begin_flip, FM_render);
    _render_callback->do_callback(&data);
  } else {
    GraphicsWindow::begin_flip();
  }
}

/**
 * This function will be called within the draw thread after begin_flip() has
 * been called on all windows, to finish the exchange of the front and back
 * buffers.
 *
 * This should cause the window to wait for the flip, if necessary.
 */
void CallbackGraphicsWindow::
end_flip() {
  if (_render_callback != nullptr) {
    RenderCallbackData data(this, RCT_end_flip, FM_render);
    _render_callback->do_callback(&data);
  } else {
    GraphicsWindow::end_flip();
  }
}

/**
 * Do whatever processing is necessary to ensure that the window responds to
 * user events.  Also, honor any requests recently made via
 * request_properties().
 *
 * This function is called only within the window thread.
 */
void CallbackGraphicsWindow::
process_events() {
  if (_events_callback != nullptr) {
    EventsCallbackData data(this);
    _events_callback->do_callback(&data);
  } else {
    GraphicsWindow::process_events();
  }
}

/**
 * Applies the requested set of properties to the window, if possible, for
 * instance to request a change in size or minimization status.
 */
void CallbackGraphicsWindow::
set_properties_now(WindowProperties &properties) {
  if (_properties_callback != nullptr) {
    PropertiesCallbackData data(this, properties);
    _properties_callback->do_callback(&data);
  } else {
    GraphicsWindow::set_properties_now(properties);
  }
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool CallbackGraphicsWindow::
open_window() {
  // In this case, we assume the callback has handled the window opening.

  // We also assume the callback has given us an accurate
  // FramebufferProperties, but we do go ahead and assume some certain minimum
  // properties.
  _fb_properties.set_rgb_color(1);

  if (_fb_properties.get_color_bits() == 0) {
    _fb_properties.set_color_bits(16);
  }
  return true;
}

/**
 * Called from the window thread in response to a request from within the code
 * (via request_properties()) to change the size and/or position of the
 * window.  Returns true if the window is successfully changed, or false if
 * there was a problem.
 */
bool CallbackGraphicsWindow::
do_reshape_request(int x_origin, int y_origin, bool has_origin,
                   int x_size, int y_size) {
  // In this case, we assume the callback has handled the window resizing.
  WindowProperties properties;
  if (has_origin) {
    properties.set_origin(x_origin, y_origin);
  }
  properties.set_size(x_size, y_size);
  system_changed_properties(properties);

  return true;
}

/**
 *
 */
void CallbackGraphicsWindow::EventsCallbackData::
upcall() {
  _window->GraphicsWindow::process_events();
}

/**
 *
 */
void CallbackGraphicsWindow::PropertiesCallbackData::
upcall() {
  _window->GraphicsWindow::set_properties_now(_properties);
}

/**
 *
 */
void CallbackGraphicsWindow::RenderCallbackData::
upcall() {
  switch (_callback_type) {
  case RCT_begin_frame:
    {
      bool render_flag = _window->GraphicsWindow::begin_frame(_frame_mode, Thread::get_current_thread());
      set_render_flag(render_flag);
    }
    break;

  case RCT_end_frame:
    _window->GraphicsWindow::end_frame(_frame_mode, Thread::get_current_thread());
    break;

  case RCT_begin_flip:
    _window->GraphicsWindow::begin_flip();
    break;

  case RCT_end_flip:
    _window->GraphicsWindow::end_flip();
    break;
  }
}
