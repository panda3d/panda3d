// Filename: sgiglxGraphicsPipe.cxx
// Created by:  cary (01Oct99)
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

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "sgiglxGraphicsPipe.h"
#include "config_sgiglxdisplay.h"

#include <glxGraphicsWindow.h>

TypeHandle SgiGlxGraphicsPipe::_type_handle;

SgiGlxGraphicsPipe::SgiGlxGraphicsPipe(const PipeSpecifier& spec)
  : sgiGraphicsPipe(spec),
    glxDisplay(this, spec.get_X_specifier())
{
}


////////////////////////////////////////////////////////////////////
//     Function: SgiGlxGraphicsPipe::get_window_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle of the kind of window
//               preferred by this kind of pipe.
////////////////////////////////////////////////////////////////////
TypeHandle SgiGlxGraphicsPipe::
get_window_type() const {
  return glxGraphicsWindow::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: SgiGlxGraphicsPipe::get_glx_display
//       Access: Public, Virtual
//  Description: Returns the glxDisplay information associated with
//               this pipe.
////////////////////////////////////////////////////////////////////
glxDisplay *SgiGlxGraphicsPipe::
get_glx_display() {
  return this;
}

GraphicsPipe *SgiGlxGraphicsPipe::
make_sgiglxgraphicspipe(const FactoryParams &params) {
  GraphicsPipe::PipeSpec *pipe_param;
  if (!get_param_into(pipe_param, params)) {
    return new SgiGlxGraphicsPipe(PipeSpecifier());
  } else {
    return new SgiGlxGraphicsPipe(pipe_param->get_specifier());
  }
}

TypeHandle SgiGlxGraphicsPipe::get_class_type(void) {
  return _type_handle;
}

void SgiGlxGraphicsPipe::init_type(void) {
  sgiGraphicsPipe::init_type();
  glxDisplay::init_type();
  register_type(_type_handle, "SgiGlxGraphicsPipe",
                sgiGraphicsPipe::get_class_type(),
                glxDisplay::get_class_type());
}

TypeHandle SgiGlxGraphicsPipe::get_type(void) const {
  return get_class_type();
}

TypeHandle SgiGlxGraphicsPipe::force_init_type(void) {
  init_type();
  return get_class_type();
}
