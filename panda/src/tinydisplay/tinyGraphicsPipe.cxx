// Filename: tinyGraphicsPipe.cxx
// Created by:  drose (24Apr08)
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

#include "tinyGraphicsPipe.h"
#include "tinyGraphicsWindow.h"
#include "tinyGraphicsStateGuardian.h"
#include "config_tinydisplay.h"
#include "frameBufferProperties.h"

TypeHandle TinyGraphicsPipe::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TinyGraphicsPipe::
TinyGraphicsPipe() {
  _is_valid = true;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    tinydisplay_cat.error()
      << "Cannot initialize SDL video.\n";
    _is_valid = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsPipe::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TinyGraphicsPipe::
~TinyGraphicsPipe() {
  if (SDL_WasInit(SDL_INIT_VIDEO)) {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
  }

  SDL_Quit();
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsPipe::get_interface_name
//       Access: Published, Virtual
//  Description: Returns the name of the rendering interface
//               associated with this GraphicsPipe.  This is used to
//               present to the user to allow him/her to choose
//               between several possible GraphicsPipes available on a
//               particular platform, so the name should be meaningful
//               and unique for a given platform.
////////////////////////////////////////////////////////////////////
string TinyGraphicsPipe::
get_interface_name() const {
  return "TinyGL";
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsPipe::pipe_constructor
//       Access: Public, Static
//  Description: This function is passed to the GraphicsPipeSelection
//               object to allow the user to make a default
//               TinyGraphicsPipe.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) TinyGraphicsPipe::
pipe_constructor() {
  return new TinyGraphicsPipe;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsPipe::make_output
//       Access: Protected, Virtual
//  Description: Creates a new window on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsOutput) TinyGraphicsPipe::
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

  TinyGraphicsStateGuardian *glxgsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(glxgsg, gsg, NULL);
  }

  // First thing to try: a TinyGraphicsWindow

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
    return new TinyGraphicsWindow(this, name, fb_prop, win_prop,
                                  flags, gsg, host);
  }
  
  // Nothing else left to try.
  return NULL;
}
