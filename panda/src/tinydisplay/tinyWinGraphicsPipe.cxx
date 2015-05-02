// Filename: tinyWinGraphicsPipe.cxx
// Created by:  drose (06May08)
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

#ifdef WIN32

#include "tinyWinGraphicsPipe.h"
#include "config_tinydisplay.h"
#include "config_windisplay.h"
#include "tinyWinGraphicsWindow.h"
#include "tinyGraphicsBuffer.h"

TypeHandle TinyWinGraphicsPipe::_type_handle;
  
////////////////////////////////////////////////////////////////////
//     Function: TinyWinGraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TinyWinGraphicsPipe::
TinyWinGraphicsPipe() {
}

////////////////////////////////////////////////////////////////////
//     Function: TinyWinGraphicsPipe::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TinyWinGraphicsPipe::
~TinyWinGraphicsPipe() {
}

////////////////////////////////////////////////////////////////////
//     Function: TinyWinGraphicsPipe::get_interface_name
//       Access: Published, Virtual
//  Description: Returns the name of the rendering interface
//               associated with this GraphicsPipe.  This is used to
//               present to the user to allow him/her to choose
//               between several possible GraphicsPipes available on a
//               particular platform, so the name should be meaningful
//               and unique for a given platform.
////////////////////////////////////////////////////////////////////
string TinyWinGraphicsPipe::
get_interface_name() const {
  return "TinyPanda";
}

////////////////////////////////////////////////////////////////////
//     Function: TinyWinGraphicsPipe::pipe_constructor
//       Access: Public, Static
//  Description: This function is passed to the GraphicsPipeSelection
//               object to allow the user to make a default
//               TinyWinGraphicsPipe.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) TinyWinGraphicsPipe::
pipe_constructor() {
  return new TinyWinGraphicsPipe;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyWinGraphicsPipe::make_output
//       Access: Protected, Virtual
//  Description: Creates a new window or buffer on the pipe, if possible.
//               This routine is only called from GraphicsEngine::make_output.
////////////////////////////////////////////////////////////////////
PT(GraphicsOutput) TinyWinGraphicsPipe::
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

  TinyGraphicsStateGuardian *tinygsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(tinygsg, gsg, NULL);
  }

  // First thing to try: a TinyWinGraphicsWindow

  if (retry == 0) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return NULL;
    }
    if ((flags & BF_fb_props_optional)==0) {
      if ((fb_prop.get_aux_rgba() > 0)||
          (fb_prop.get_aux_hrgba() > 0)||
          (fb_prop.get_aux_float() > 0)) {
        return NULL;
      }
    }
    return new TinyWinGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                     flags, gsg, host);
  }
  
  // Second thing to try: a TinyGraphicsBuffer
  if (retry == 1) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)) {
      return NULL;
    }
    return new TinyGraphicsBuffer(engine, this, name, fb_prop, win_prop, 
                                  flags, gsg, host);
  }
  
  // Nothing else left to try.
  return NULL;
}

#endif  // WIN32
