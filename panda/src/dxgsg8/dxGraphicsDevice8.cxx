// Filename: dxGraphicsDevice.cxx
// Created by:  masad (22Jul03)
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

#include "config_dxgsg8.h"
#include "dxGraphicsDevice8.h"


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsDevice8::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXGraphicsDevice8::
DXGraphicsDevice8(wdxGraphicsPipe8 *pipe) :
  GraphicsDevice(pipe) {

  ZeroMemory(&_Scrn,sizeof(_Scrn));
  _d3d_device = NULL;
  _swap_chain = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsDevice8::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXGraphicsDevice8::
~DXGraphicsDevice8() {

}

