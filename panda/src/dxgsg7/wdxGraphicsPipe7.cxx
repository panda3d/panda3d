// Filename: wdxGraphicsPipe7.cxx
// Created by:  drose (20Dec02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "wdxGraphicsPipe7.h"
#include "wdxGraphicsWindow7.h"
#include "config_dxgsg7.h"

TypeHandle wdxGraphicsPipe7::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipe7::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
wdxGraphicsPipe7::
wdxGraphicsPipe7() {
  _hDDrawDLL = NULL;
  _is_valid = init();
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipe7::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
wdxGraphicsPipe7::
~wdxGraphicsPipe7() {
  SAFE_FREELIB(_hDDrawDLL);
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipe7::get_interface_name
//       Access: Published, Virtual
//  Description: Returns the name of the rendering interface
//               associated with this GraphicsPipe.  This is used to
//               present to the user to allow him/her to choose
//               between several possible GraphicsPipes available on a
//               particular platform, so the name should be meaningful
//               and unique for a given platform.
////////////////////////////////////////////////////////////////////
string wdxGraphicsPipe7::
get_interface_name() const {
  return "DirectX7";
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipe7::pipe_constructor
//       Access: Public, Static
//  Description: This function is passed to the GraphicsPipeSelection
//               object to allow the user to make a default
//               wdxGraphicsPipe7.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) wdxGraphicsPipe7::
pipe_constructor() {
  return new wdxGraphicsPipe7;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipe7::make_window
//       Access: Protected, Virtual
//  Description: Creates a new window on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsWindow) wdxGraphicsPipe7::
make_window(GraphicsStateGuardian *gsg) {
  // thanks to the dumb threading requirements this constructor actually does nothing but create an empty c++ object
  // no windows are really opened until wdxGraphicsWindow8->open_window() is called

  return new wdxGraphicsWindow7(this, gsg);
}


PT(GraphicsStateGuardian) wdxGraphicsPipe7::
make_gsg(const FrameBufferProperties &properties) {

  // FrameBufferProperties really belongs as part of the window/renderbuffer specification
  // put here because of GLX multithreading requirement
  PT(DXGraphicsStateGuardian7) gsg = new DXGraphicsStateGuardian7(properties);
  return gsg.p();
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipe7::init
//       Access: Private
//  Description: Performs some initialization steps to load up
//               function pointers from the relevant DLL's, and
//               determine the number and type of available graphics
//               adapters, etc.  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsPipe7::
init() {

  if(!MyLoadLib(_hDDrawDLL,"ddraw.dll")) {
      goto error;
  }

  if(!MyGetProcAddr(_hDDrawDLL, (FARPROC*)&_DirectDrawCreateEx, "DirectDrawCreateEx")) {
      goto error;
  }

  return true;

error:
  return false;  
}
