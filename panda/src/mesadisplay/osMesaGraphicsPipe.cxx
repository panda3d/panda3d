// Filename: osMesaGraphicsPipe.cxx
// Created by:  drose (09Feb04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "osMesaGraphicsPipe.h"
#include "osMesaGraphicsBuffer.h"
#include "osMesaGraphicsStateGuardian.h"
#include "config_mesadisplay.h"
#include "frameBufferProperties.h"
#include "mutexHolder.h"

TypeHandle OsMesaGraphicsPipe::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: OsMesaGraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
OsMesaGraphicsPipe::
OsMesaGraphicsPipe() {
  _supported_types = OT_buffer | OT_texture_buffer;
  _is_valid = true;
}

////////////////////////////////////////////////////////////////////
//     Function: OsMesaGraphicsPipe::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
OsMesaGraphicsPipe::
~OsMesaGraphicsPipe() {
}

////////////////////////////////////////////////////////////////////
//     Function: OsMesaGraphicsPipe::get_interface_name
//       Access: Published, Virtual
//  Description: Returns the name of the rendering interface
//               associated with this GraphicsPipe.  This is used to
//               present to the user to allow him/her to choose
//               between several possible GraphicsPipes available on a
//               particular platform, so the name should be meaningful
//               and unique for a given platform.
////////////////////////////////////////////////////////////////////
string OsMesaGraphicsPipe::
get_interface_name() const {
  return "Offscreen Mesa";
}

////////////////////////////////////////////////////////////////////
//     Function: OsMesaGraphicsPipe::pipe_constructor
//       Access: Public, Static
//  Description: This function is passed to the GraphicsPipeSelection
//               object to allow the user to make a default
//               OsMesaGraphicsPipe.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) OsMesaGraphicsPipe::
pipe_constructor() {
  return new OsMesaGraphicsPipe;
}

////////////////////////////////////////////////////////////////////
//     Function: OsMesaGraphicsPipe::make_output
//       Access: Protected, Virtual
//  Description: Creates a new window on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsOutput) OsMesaGraphicsPipe::
make_output(const string &name,
            const FrameBufferProperties &fb_prop,
            const WindowProperties &win_prop,
            int flags,
            GraphicsEngine *engine,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool &precertify) {
  
  if (!_is_valid) {
    return NULL;
  }

  if (retry == 0) {
    if ((!support_render_texture)||
        ((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_can_bind_every)!=0)||
        ((flags&BF_rtt_cumulative)!=0)) {
      return NULL;
    }

    // We know that OsMesa windows only support single buffering.  So
    // don't bother asking for more than that.
    FrameBufferProperties fb_prop2 = fb_prop;
    fb_prop2.set_back_buffers(0);

    return new OsMesaGraphicsBuffer(engine, this, name, fb_prop2, win_prop,
                                    flags, gsg, host);
  }
  
  // Nothing else left to try.
  return NULL;
}
