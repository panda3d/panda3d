// Filename: dxGraphicsDevice.h
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

#ifndef DXGRAPHICSDEVICE_H
#define DXGRAPHICSDEVICE_H

//#define GSG_VERBOSE 1

#include "dxgsg9base.h"
#include "graphicsDevice.h"
#include "wdxGraphicsPipe9.h"


////////////////////////////////////////////////////////////////////
//   Class : DXGraphicsDevice9
// Description : A GraphicsDevice necessary for multi-window rendering
//               in DX.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXGraphicsDevice9 : public GraphicsDevice {
  friend class wdxGraphicsPipe9;

public:
  DXGraphicsDevice9(wdxGraphicsPipe9 *pipe);
  ~DXGraphicsDevice9();

  DXScreenData _Scrn;
  LPDIRECT3DDEVICE9 _d3d_device;  // same as Scrn._d3d_device, cached for spd
  IDirect3DSwapChain9 *_swap_chain;

#if 0
protected:
  static TypeHandle get_class_type();
  static void init_type();
  virtual TypeHandle get_type() const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
#endif
};

#endif
