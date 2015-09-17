// Filename: graphicsPipe.cxx
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

#include "graphicsPipe.h"
#include "graphicsWindow.h"
#include "graphicsBuffer.h"
#include "config_display.h"
#include "mutexHolder.h"
#include "displayInformation.h"

TypeHandle GraphicsPipe::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsPipe::
GraphicsPipe() :
  _lock("GraphicsPipe")
{
  // Initially, we assume the GraphicsPipe is valid.  A derived class
  // should set this to false if it determines otherwise.
  _is_valid = true;

  // A derived class must indicate the kinds of GraphicsOutput objects
  // it can create.
  _supported_types = 0;

  _display_width = 0;
  _display_height = 0;

  _display_information = new DisplayInformation ( );
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::Copy Constructor
//       Access: Private
//  Description: Don't try to copy GraphicsPipes.
////////////////////////////////////////////////////////////////////
GraphicsPipe::
GraphicsPipe(const GraphicsPipe &) {
  _is_valid = false;
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::Copy Assignment Operator
//       Access: Private
//  Description: Don't try to copy GraphicsPipes.
////////////////////////////////////////////////////////////////////
void GraphicsPipe::
operator = (const GraphicsPipe &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsPipe::
~GraphicsPipe() {
  delete _display_information;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_preferred_window_thread
//       Access: Public, Virtual
//  Description: Returns an indication of the thread in which this
//               GraphicsPipe requires its window processing to be
//               performed: typically either the app thread (e.g. X)
//               or the draw thread (Windows).
////////////////////////////////////////////////////////////////////
GraphicsPipe::PreferredWindowThread
GraphicsPipe::get_preferred_window_thread() const {
  return PWT_draw;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::make_callback_gsg
//       Access: Public, Virtual
//  Description: This is called when make_output() is used to create a
//               CallbackGraphicsWindow.  If the GraphicsPipe can
//               construct a GSG that's not associated with any
//               particular window object, do so now, assuming the
//               correct graphics context has been set up externally.
////////////////////////////////////////////////////////////////////
PT(GraphicsStateGuardian) GraphicsPipe::
make_callback_gsg(GraphicsEngine *engine) {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::make_device
//       Access: Public, Virtual
//  Description: Creates a new device for the pipe. Only DirectX uses
//               this device, for other api's it is NULL.
////////////////////////////////////////////////////////////////////
PT(GraphicsDevice) GraphicsPipe::
make_device(void *scrn) {
  display_cat.error()
    << "make_device() unimplemented by " << get_type() << "\n";
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::close_gsg
//       Access: Protected, Virtual
//  Description: This will be called in the draw thread (the same
//               thread in which the GSG was created via make_gsg,
//               above) to close the indicated GSG and free its
//               associated graphics objects just before it is
//               destructed.  This method exists to provide a hook for
//               the graphics pipe to do any necessary cleanup, if
//               any.
////////////////////////////////////////////////////////////////////
void GraphicsPipe::
close_gsg(GraphicsStateGuardian *gsg) {
  if (gsg != (GraphicsStateGuardian *)NULL) {
    gsg->close_gsg();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::make_output
//       Access: Protected, Virtual
//  Description: Creates a new window on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsOutput) GraphicsPipe::
make_output(const string &name,
            const FrameBufferProperties &fb_prop,
            const WindowProperties &win_prop,
            int flags,
            GraphicsEngine *engine,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool &precertify) {
  display_cat.error()
    << get_type() << " cannot create buffers or windows.\n";
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_display_information
//       Access: Published
//  Description: Gets the pipe's DisplayInformation.
////////////////////////////////////////////////////////////////////
DisplayInformation *GraphicsPipe::
get_display_information() {
  return _display_information;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::lookup_cpu_data
//       Access: Public, Virtual
//  Description: Looks up the detailed CPU information and stores it
//               in _display_information, if supported by the OS.
//               This may take a second or two.
////////////////////////////////////////////////////////////////////
void GraphicsPipe::
lookup_cpu_data() {
}
