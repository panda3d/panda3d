// Filename: glxGraphicsPipe.cxx
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
#include "glxGraphicsPipe.h"
#include "config_glxdisplay.h"

#include <GL/glx.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle glxGraphicsPipe::_type_handle;

glxGraphicsPipe::glxGraphicsPipe(const PipeSpecifier& spec)
  : InteractiveGraphicsPipe(spec),
    glxDisplay(this, spec.get_X_specifier())
{
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::get_window_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle of the kind of window
//               preferred by this kind of pipe.
////////////////////////////////////////////////////////////////////
TypeHandle glxGraphicsPipe::
get_window_type() const {
  return glxGraphicsWindow::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::get_glx_display
//       Access: Public, Virtual
//  Description: Returns the glxDisplay information associated with
//               this pipe.
////////////////////////////////////////////////////////////////////
glxDisplay *glxGraphicsPipe::
get_glx_display() {
  return this;
}

GraphicsPipe *glxGraphicsPipe::
make_glxGraphicsPipe(const FactoryParams &params) {
  GraphicsPipe::PipeSpec *pipe_param;
  if (!get_param_into(pipe_param, params)) {
    return new glxGraphicsPipe(PipeSpecifier());
  } else {
    return new glxGraphicsPipe(pipe_param->get_specifier());
  }
}

TypeHandle glxGraphicsPipe::get_class_type(void) {
  return _type_handle;
}

void glxGraphicsPipe::init_type(void) {
  InteractiveGraphicsPipe::init_type();
  glxDisplay::init_type();
  register_type(_type_handle, "glxGraphicsPipe",
                InteractiveGraphicsPipe::get_class_type(),
                glxDisplay::get_class_type());
}

TypeHandle glxGraphicsPipe::get_type(void) const {
  return get_class_type();
}
