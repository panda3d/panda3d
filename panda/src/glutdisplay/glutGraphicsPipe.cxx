// Filename: glutGraphicsPipe.cxx
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
#include "glutGraphicsPipe.h"
#include "config_glutdisplay.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle glutGraphicsPipe::_type_handle;

glutGraphicsPipe::glutGraphicsPipe(const PipeSpecifier& spec)
  : InteractiveGraphicsPipe(spec) {}

////////////////////////////////////////////////////////////////////
//     Function: glutGraphicsPipe::get_window_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle of the kind of window
//               preferred by this kind of pipe.
////////////////////////////////////////////////////////////////////
TypeHandle glutGraphicsPipe::
get_window_type() const {
  return glutGraphicsWindow::get_class_type();
}

GraphicsPipe*
glutGraphicsPipe::make_glutGraphicsPipe(const FactoryParams &params) {
  GraphicsPipe::PipeSpec *pipe_param;
  if (!get_param_into(pipe_param, params)) {
    return new glutGraphicsPipe(PipeSpecifier());
  } else {
    return new glutGraphicsPipe(pipe_param->get_specifier());
  }
}

TypeHandle glutGraphicsPipe::get_class_type(void) {
  return _type_handle;
}

void glutGraphicsPipe::init_type(void) {
  InteractiveGraphicsPipe::init_type();
  register_type(_type_handle, "glutGraphicsPipe",
                InteractiveGraphicsPipe::get_class_type());
}

TypeHandle glutGraphicsPipe::get_type(void) const {
  return get_class_type();
}

glutGraphicsPipe::glutGraphicsPipe(void) {
  glutdisplay_cat.error()
    << "glutGraphicsPipes should not be created with the default constructor"
    << endl;
}

glutGraphicsPipe::glutGraphicsPipe(const glutGraphicsPipe&) {
  glutdisplay_cat.error()
    << "glutGraphicsPipes should not be copied" << endl;
}

glutGraphicsPipe& glutGraphicsPipe::operator=(const glutGraphicsPipe&) {
  glutdisplay_cat.error()
    << "glutGraphicsPipes should not be assigned" << endl;
  return *this;
}
