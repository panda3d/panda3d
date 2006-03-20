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

#include "osxGraphicsPipe.h"
#include "config_osxdisplay.h"
#include "osxGraphicsWindow.h"
#include "osxGraphicsBuffer.h"
#include "osxGraphicsStateGuardian.h"


TypeHandle osxGraphicsPipe::_type_handle;
  
////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
osxGraphicsPipe::
osxGraphicsPipe() {
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsPipe::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
osxGraphicsPipe::
~osxGraphicsPipe() {
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsPipe::get_interface_name
//       Access: Published, Virtual
//  Description: Returns the name of the rendering interface
//               associated with this GraphicsPipe.  This is used to
//               present to the user to allow him/her to choose
//               between several possible GraphicsPipes available on a
//               particular platform, so the name should be meaningful
//               and unique for a given platform.
////////////////////////////////////////////////////////////////////
string osxGraphicsPipe::
get_interface_name() const {
  return "OpenGL";
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsPipe::pipe_constructor
//       Access: Public, Static
//  Description: This function is passed to the GraphicsPipeSelection
//               object to allow the user to make a default
//               osxGraphicsPipe.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) osxGraphicsPipe::
pipe_constructor() {
  return new osxGraphicsPipe;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsPipe::make_gsg
//       Access: Protected, Virtual
//  Description: Creates a new GSG to use the pipe (but no windows
//               have been created yet for the GSG).  This method will
//               be called in the draw thread for the GSG.
////////////////////////////////////////////////////////////////////
PT(GraphicsStateGuardian) osxGraphicsPipe::
make_gsg(const FrameBufferProperties &properties, 
         GraphicsStateGuardian *share_with) {
		 
		 
  if (!_is_valid) {
    return NULL;
  }

  osxGraphicsStateGuardian *share_gsg = NULL;

  if (share_with != (GraphicsStateGuardian *)NULL) {
    if (!share_with->is_exact_type(osxGraphicsStateGuardian::get_class_type())) 
	{
      osxdisplay_cat.error()
        << "Cannot share context between osxGraphicsStateGuardian and "
        << share_with->get_type() << "\n";
      return NULL;
    }

    DCAST_INTO_R(share_gsg, share_with, NULL);
  }

  int frame_buffer_mode = 0;
  if (properties.has_frame_buffer_mode()) {
    frame_buffer_mode = properties.get_frame_buffer_mode();
  }


	PT(osxGraphicsStateGuardian) gsg1 = new osxGraphicsStateGuardian(properties,(osxGraphicsStateGuardian *)  share_with, 1);
                          
    return gsg1.p();
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsPipe::make_output
//       Access: Protected, Virtual
//  Description: Creates a new window on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsOutput) osxGraphicsPipe::
make_output(const string &name,
            int x_size, int y_size, int flags,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool precertify) {
  
  if (!_is_valid) {
    return NULL;
  }

  osxGraphicsStateGuardian *osxgsg;
  DCAST_INTO_R(osxgsg, gsg, NULL);

  // First thing to try: a osxGraphicsWindow

  if (retry == 0) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_need_aux_rgba_MASK)!=0)||
        ((flags&BF_need_aux_hrgba_MASK)!=0)||
        ((flags&BF_need_aux_float_MASK)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return NULL;
    }
    return new osxGraphicsWindow(this, name, x_size, y_size, flags, gsg, host);
  }
  
  //  // Second thing to try: a glGraphicsBuffer
  //  
  //  if (retry == 1) {
  //    if ((!support_render_texture)||
  //        ((flags&BF_require_parasite)!=0)||
  //        ((flags&BF_require_window)!=0)) {
  //      return NULL;
  //    }
  //    if (precertify) {
  //      if (!osxgsg->_supports_framebuffer_object) {
  //        return NULL;
  //      }
  //    }
  //    return new glGraphicsBuffer(this, name, x_size, y_size, flags, gsg, host);
  //  }
  
  // Third thing to try: an osxGraphicsBuffer
  
  if (retry == 2) {
    if ((!support_render_texture)||
        ((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)||
        ((flags&BF_need_aux_rgba_MASK)!=0)||
        ((flags&BF_need_aux_hrgba_MASK)!=0)||
        ((flags&BF_need_aux_float_MASK)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return NULL;
    }
    return new osxGraphicsBuffer(this, name, x_size, y_size, flags, gsg, host);
  }
  
  // Nothing else left to try.
  return NULL;
}

