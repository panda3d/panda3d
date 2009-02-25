// Filename: tinyOffscreenGraphicsPipe.cxx
// Created by:  drose (09Feb09)
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

#include "pandabase.h"

#include "tinyOffscreenGraphicsPipe.h"
#include "tinyGraphicsStateGuardian.h"
#include "tinyGraphicsBuffer.h"
#include "config_tinydisplay.h"
#include "frameBufferProperties.h"

TypeHandle TinyOffscreenGraphicsPipe::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TinyOffscreenGraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TinyOffscreenGraphicsPipe::
TinyOffscreenGraphicsPipe() {
  _supported_types = OT_buffer | OT_texture_buffer;
  _is_valid = true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOffscreenGraphicsPipe::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TinyOffscreenGraphicsPipe::
~TinyOffscreenGraphicsPipe() {
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOffscreenGraphicsPipe::get_interface_name
//       Access: Published, Virtual
//  Description: Returns the name of the rendering interface
//               associated with this GraphicsPipe.  This is used to
//               present to the user to allow him/her to choose
//               between several possible GraphicsPipes available on a
//               particular platform, so the name should be meaningful
//               and unique for a given platform.
////////////////////////////////////////////////////////////////////
string TinyOffscreenGraphicsPipe::
get_interface_name() const {
  return "TinyPanda";
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOffscreenGraphicsPipe::pipe_constructor
//       Access: Public, Static
//  Description: This function is passed to the GraphicsPipeSelection
//               object to allow the user to make a default
//               TinyOffscreenGraphicsPipe.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) TinyOffscreenGraphicsPipe::
pipe_constructor() {
  return new TinyOffscreenGraphicsPipe;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOffscreenGraphicsPipe::make_output
//       Access: Protected, Virtual
//  Description: Creates a new window on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsOutput) TinyOffscreenGraphicsPipe::
make_output(const string &name,
            const FrameBufferProperties &fb_prop,
            const WindowProperties &win_prop,
            int flags,
            GraphicsEngine *engine,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool &precertify) {
  // Only thing to try: a TinyGraphicsBuffer

  if (retry == 0) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)) {
      return NULL;
    }
    return new TinyGraphicsBuffer(engine, this, name, fb_prop, win_prop, flags, gsg, host);
  }
  
  // Nothing else left to try.
  return NULL;
}
