// Filename: ribGraphicsPipe.cxx
// Created by:  drose (15Feb99)
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
