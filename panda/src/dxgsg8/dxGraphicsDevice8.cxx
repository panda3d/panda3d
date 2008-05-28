// Filename: dxGraphicsDevice.cxx
// Created by:  masad (22Jul03)
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

