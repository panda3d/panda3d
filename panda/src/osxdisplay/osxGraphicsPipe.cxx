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
//     Function: osxGraphicsPipe::make_output
//       Access: Protected, Virtual
//  Description: Creates a new window on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsOutput) osxGraphicsPipe::
make_output(const string &name,
            const FrameBufferProperties &fb_prop,
            const WindowProperties &win_prop,
            int flags,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool &precertify) {
  
  if (!_is_valid) {
    return NULL;
  }
  
  osxGraphicsStateGuardian *osxgsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(osxgsg, gsg, NULL);
  }
  
  // First thing to try: a osxGraphicsWindow

  if (retry == 0) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return NULL;
    }
    return new osxGraphicsWindow(this, name, fb_prop, win_prop,
                                 flags, gsg, host);
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
  //    return new glGraphicsBuffer(this, name, fb_prop, win_prop, flags, gsg, host);
  //  }
  
  // Third thing to try: an osxGraphicsBuffer
 /* 
  if (retry == 2) {
    if ((!support_render_texture)||
        ((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return NULL;
    }
    return new osxGraphicsBuffer(this, name, fb_prop, win_prop,
                                 flags, gsg, host);
  }
  */
  // Nothing else left to try.
  return NULL;
}

