// Filename: ribGraphicsPipe.cxx
// Created by:  drose (15Feb99)
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

#include "ribGraphicsPipe.h"
#include "ribGraphicsWindow.h"
#include "config_ribdisplay.h"

TypeHandle RIBGraphicsPipe::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsPipe::get_window_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle of the kind of window
//               preferred by this kind of pipe.
////////////////////////////////////////////////////////////////////
TypeHandle RIBGraphicsPipe::
get_window_type() const {
  return RIBGraphicsWindow::get_class_type();
}


GraphicsPipe *RIBGraphicsPipe::
make_RIBGraphicsPipe(const FactoryParams &params) {
  GraphicsPipe::PipeSpec *pipe_param;
  if (!get_param_into(pipe_param, params)) {
    return new RIBGraphicsPipe(PipeSpecifier());
  } else {
    return new RIBGraphicsPipe(pipe_param->get_specifier());
  }
}

TypeHandle RIBGraphicsPipe::get_class_type(void) {
  return _type_handle;
}

void RIBGraphicsPipe::init_type(void) {
  NoninteractiveGraphicsPipe::init_type();
  register_type(_type_handle, "RIBGraphicsPipe",
                NoninteractiveGraphicsPipe::get_class_type());
}

TypeHandle RIBGraphicsPipe::get_type(void) const {
  return get_class_type();
}
