// Filename: sgiglxGraphicsPipe.cxx
// Created by:  cary (01Oct99)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "sgiglxGraphicsPipe.h"
#include "config_sgiglxdisplay.h"

TypeHandle SgiGlxGraphicsPipe::_type_handle;

SgiGlxGraphicsPipe::SgiGlxGraphicsPipe(const PipeSpecifier& spec)
  : sgiGraphicsPipe(spec)
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
SgiGlxDisplay *glxGraphicsPipe::
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
