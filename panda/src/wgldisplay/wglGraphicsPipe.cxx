// Filename: wglGraphicsPipe.cxx
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
#include "wglGraphicsPipe.h"
#include "config_wgldisplay.h"
#include <mouseButton.h>
#include <keyboardButton.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle wglGraphicsPipe::_type_handle;

//wglGraphicsPipe *global_pipe;

wglGraphicsPipe::wglGraphicsPipe(const PipeSpecifier& spec)
  : InteractiveGraphicsPipe(spec)
{}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsPipe::get_window_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle of the kind of window
//               preferred by this kind of pipe.
////////////////////////////////////////////////////////////////////
TypeHandle wglGraphicsPipe::
get_window_type() const {
  return wglGraphicsWindow::get_class_type();
}

GraphicsPipe *wglGraphicsPipe::
make_wglGraphicsPipe(const FactoryParams &params) {
  GraphicsPipe::PipeSpec *pipe_param;
  if (!get_param_into(pipe_param, params)) {
    return new wglGraphicsPipe(PipeSpecifier());
  } else {
    return new wglGraphicsPipe(pipe_param->get_specifier());
  }
}


TypeHandle wglGraphicsPipe::get_class_type(void) {
  return _type_handle;
}

void wglGraphicsPipe::init_type(void) {
  InteractiveGraphicsPipe::init_type();
  register_type(_type_handle, "wglGraphicsPipe",
        InteractiveGraphicsPipe::get_class_type());
}

TypeHandle wglGraphicsPipe::get_type(void) const {
  return get_class_type();
}

wglGraphicsPipe::wglGraphicsPipe(void) {
  wgldisplay_cat.error()
    << "wglGraphicsPipes should not be created with the default constructor"
    << endl;
}

wglGraphicsPipe::wglGraphicsPipe(const wglGraphicsPipe&) {
  wgldisplay_cat.error()
    << "wglGraphicsPipes should not be copied" << endl;
}

wglGraphicsPipe& wglGraphicsPipe::operator=(const wglGraphicsPipe&) {
  wgldisplay_cat.error()
    << "wglGraphicsPipes should not be assigned" << endl;
  return *this;
}
