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
  if (_hDDrawDLL != NULL) {
    FreeLibrary(_hDDrawDLL);
    _hDDrawDLL = NULL;
  }
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
make_window() {
  return new wdxGraphicsWindow7(this);
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
  static const char * const ddraw_name = "ddraw.dll";
  _hDDrawDLL = LoadLibrary(ddraw_name);
  if(_hDDrawDLL == 0) {
    wdxdisplay7_cat.error()
      << "can't locate " << ddraw_name << "!\n";
    return false;
  }

  _DirectDrawCreateEx = 
    (LPDIRECTDRAWCREATEEX)GetProcAddress(_hDDrawDLL, "DirectDrawCreateEx");
  if (_DirectDrawCreateEx == NULL) {
    wdxdisplay7_cat.error()
      << "GetProcAddr failed for DDCreateEx" << endl;
    return false;
  }

  return true;
}
