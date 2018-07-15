/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxGraphicsDevice9.cxx
 * @author masad
 * @date 2003-07-22
 */

#include "config_dxgsg9.h"
#include "dxGraphicsDevice9.h"


/**
 *
 */
DXGraphicsDevice9::
DXGraphicsDevice9(wdxGraphicsPipe9 *pipe) :
  GraphicsDevice(pipe) {

  ZeroMemory(&_Scrn,sizeof(_Scrn));
  _d3d_device = nullptr;
  _swap_chain = nullptr;
}

/**
 *
 */
DXGraphicsDevice9::
~DXGraphicsDevice9() {
}
