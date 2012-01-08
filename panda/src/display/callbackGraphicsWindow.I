// Filename: callbackGraphicsWindow.I
// Created by:  drose (06Jan11)
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
//     Function: CallbackGraphicsWindow::set_events_callback
//       Access: Published
//  Description: Sets the CallbackObject that will be notified when
//               this window is polled for window events, including
//               mouse and keyboard events, as well as window resize
//               events and other system-generated events.
//
//               This callback will receive a
//               CallbackGraphicsWindow::EventsCallbackData.
//
//               This callback should process any system-generated
//               events, and call data->upcall() to process requested
//               property change requests made via
//               request_properties().
////////////////////////////////////////////////////////////////////
INLINE void CallbackGraphicsWindow::
set_events_callback(CallbackObject *object) {
  _events_callback = object;
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::clear_events_callback
//       Access: Published
//  Description: Removes the callback set by an earlier call to
//               set_events_callback().
////////////////////////////////////////////////////////////////////
INLINE void CallbackGraphicsWindow::
clear_events_callback() {
  set_events_callback(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::get_events_callback
//       Access: Published
//  Description: Returns the CallbackObject set by set_events_callback().
////////////////////////////////////////////////////////////////////
INLINE CallbackObject *CallbackGraphicsWindow::
get_events_callback() const {
  return _events_callback;
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::set_properties_callback
//       Access: Published
//  Description: Sets the CallbackObject that will be notified when
//               this window receives a property change request from
//               user code (e.g. via request_properties).
//
//               This callback will receive a
//               CallbackGraphicsWindow::PropertiesCallbackData, which
//               provides a get_properties() method that returns a
//               modifiable reference to a WindowsProperties object.
//               This object will contain only those properties
//               requested by user code.  The callback should handle
//               any of the requests it finds, including and
//               especially set_open(), and remove them from the
//               object when it has handled them.  Any unhandled
//               properties should be left unchanged in the properties
//               object.
////////////////////////////////////////////////////////////////////
INLINE void CallbackGraphicsWindow::
set_properties_callback(CallbackObject *object) {
  _properties_callback = object;
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::clear_properties_callback
//       Access: Published
//  Description: Removes the callback set by an earlier call to
//               set_properties_callback().
////////////////////////////////////////////////////////////////////
INLINE void CallbackGraphicsWindow::
clear_properties_callback() {
  set_properties_callback(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::get_properties_callback
//       Access: Published
//  Description: Returns the CallbackObject set by set_properties_callback().
////////////////////////////////////////////////////////////////////
INLINE CallbackObject *CallbackGraphicsWindow::
get_properties_callback() const {
  return _properties_callback;
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::set_render_callback
//       Access: Published
//  Description: Sets the CallbackObject that will be notified when
//               this window is invoked (in the draw thread) to render
//               its contents, and/or flip the graphics buffers.
//
//               This callback will actually serve several different
//               functions.  It receivces a RenderCallbackData, and
//               you can query data->get_callback_type() to return the
//               actual function of each particular callback.
////////////////////////////////////////////////////////////////////
INLINE void CallbackGraphicsWindow::
set_render_callback(CallbackObject *object) {
  _render_callback = object;
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::clear_render_callback
//       Access: Published
//  Description: Removes the callback set by an earlier call to
//               set_render_callback().
////////////////////////////////////////////////////////////////////
INLINE void CallbackGraphicsWindow::
clear_render_callback() {
  set_render_callback(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::get_render_callback
//       Access: Published
//  Description: Returns the CallbackObject set by set_render_callback().
////////////////////////////////////////////////////////////////////
INLINE CallbackObject *CallbackGraphicsWindow::
get_render_callback() const {
  return _render_callback;
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::WindowCallbackData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CallbackGraphicsWindow::WindowCallbackData::
WindowCallbackData(CallbackGraphicsWindow *window) : _window(window) {
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::WindowCallbackData::get_window
//       Access: Published
//  Description: Returns the window this callback was triggered from.
////////////////////////////////////////////////////////////////////
INLINE CallbackGraphicsWindow *CallbackGraphicsWindow::WindowCallbackData::
get_window() const {
  return _window;
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::EventsCallbackData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CallbackGraphicsWindow::EventsCallbackData::
EventsCallbackData(CallbackGraphicsWindow *window) :
  WindowCallbackData(window)
{
}


////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::PropertiesCallbackData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CallbackGraphicsWindow::PropertiesCallbackData::
PropertiesCallbackData(CallbackGraphicsWindow *window, WindowProperties &properties) :
  WindowCallbackData(window),
  _properties(properties)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::PropertiesCallbackData::get_properties
//       Access: Published
//  Description: Returns the WindowProperties object that this
//               callback should process.  Any properties that are
//               handled should be removed from this object;
//               properties that are unhandled should be left alone.
////////////////////////////////////////////////////////////////////
INLINE WindowProperties &CallbackGraphicsWindow::PropertiesCallbackData::
get_properties() const {
  return _properties;
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::RenderCallbackData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CallbackGraphicsWindow::RenderCallbackData::
RenderCallbackData(CallbackGraphicsWindow *window, RenderCallbackType callback_type, FrameMode frame_mode) : 
  WindowCallbackData(window),
  _callback_type(callback_type),
  _frame_mode(frame_mode),
  _render_flag(true)
{
}
  
////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::RenderCallbackData::get_callback_type
//       Access: Published
//  Description: Since the render callback is shared for several
//               functions, this method is needed to indicate which
//               particular function is being invoked with this
//               callback.
////////////////////////////////////////////////////////////////////
INLINE CallbackGraphicsWindow::RenderCallbackType CallbackGraphicsWindow::RenderCallbackData::
get_callback_type() const {
  return _callback_type;
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::RenderCallbackData::get_frame_mode
//       Access: Published
//  Description: If the callback type (returned by get_callback_type)
//               is RCT_begin_frame or RCT_end_frame, then this method
//               will return the particular frame mode indicating
//               what, precisely, we want to do this frame.
////////////////////////////////////////////////////////////////////
INLINE GraphicsOutput::FrameMode CallbackGraphicsWindow::RenderCallbackData::
get_frame_mode() const {
  return _frame_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::RenderCallbackData::set_render_flag
//       Access: Published
//  Description: If the callback type is RCT_begin_frame, this call is
//               available to specify the return value from the
//               begin_frame() call.  If this is true (the default),
//               the frame is rendered normally; if it is false, the
//               frame is omitted.
////////////////////////////////////////////////////////////////////
INLINE void CallbackGraphicsWindow::RenderCallbackData::
set_render_flag(bool render_flag) {
  _render_flag = render_flag;
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackGraphicsWindow::RenderCallbackData::get_render_flag
//       Access: Published
//  Description: Returns the current setting of the render flag.  See
//               set_render_flag().
////////////////////////////////////////////////////////////////////
INLINE bool CallbackGraphicsWindow::RenderCallbackData::
get_render_flag() const {
  return _render_flag;
}
