// Filename: wcrGraphicsPipe.cxx
// Created by:  skyler, based on wgl* file.
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////
#include "wcrGraphicsPipe.h"
#include "config_wcrdisplay.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle wcrGraphicsPipe::_type_handle;

wcrGraphicsPipe::wcrGraphicsPipe(const PipeSpecifier& spec)
  : InteractiveGraphicsPipe(spec)
{}

////////////////////////////////////////////////////////////////////
//     Function: wcrGraphicsPipe::get_window_type
//       Access: Published, Virtual
//  Description: Returns the TypeHandle of the kind of window
//               preferred by this kind of pipe.
////////////////////////////////////////////////////////////////////
TypeHandle wcrGraphicsPipe::
get_window_type() const {
  return wcrGraphicsWindow::get_class_type();
}

GraphicsPipe *wcrGraphicsPipe::
make_wcrGraphicsPipe(const FactoryParams &params) {
  GraphicsPipe::PipeSpec *pipe_param;
  if (!get_param_into(pipe_param, params)) {
    return new wcrGraphicsPipe(PipeSpecifier());
  } else {
    return new wcrGraphicsPipe(pipe_param->get_specifier());
  }
}


TypeHandle wcrGraphicsPipe::get_class_type() {
  return _type_handle;
}

const char *pipe_type_name="wcrGraphicsPipe";

void wcrGraphicsPipe::init_type() {
  InteractiveGraphicsPipe::init_type();
  register_type(_type_handle, pipe_type_name,
        InteractiveGraphicsPipe::get_class_type());
}

TypeHandle wcrGraphicsPipe::get_type() const {
  return get_class_type();
}

wcrGraphicsPipe::wcrGraphicsPipe() {
  wcrdisplay_cat.error()
    << pipe_type_name <<"s should not be created with the default constructor" << endl;
}

wcrGraphicsPipe::wcrGraphicsPipe(const wcrGraphicsPipe&) {
  wcrdisplay_cat.error()
    << pipe_type_name << "s should not be copied" << endl;
}

wcrGraphicsPipe& wcrGraphicsPipe::operator=(const wcrGraphicsPipe&) {
  wcrdisplay_cat.error() 
  << pipe_type_name << "s should not be assigned" << endl;
  return *this;
}
