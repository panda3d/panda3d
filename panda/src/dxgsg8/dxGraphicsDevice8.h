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

#include "dxgsg8base.h"
#include "graphicsDevice.h"
#include "wdxGraphicsPipe8.h"


////////////////////////////////////////////////////////////////////
//   Class : DXGraphicsDevice8
// Description : A GraphicsDevice necessary for multi-window rendering
//               in DX. 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXGraphicsDevice8 : public GraphicsDevice {
  friend class wdxGraphicsPipe8;

public:
  DXGraphicsDevice8(wdxGraphicsPipe8 *pipe);
  ~DXGraphicsDevice8();

  DXScreenData _Scrn;
  LPDIRECT3DDEVICE8 _d3d_device;  // same as Scrn._d3d_device, cached for spd
  IDirect3DSwapChain8 *_swap_chain;

#if 0
protected:
  static TypeHandle get_class_type();
  static void init_type();
  virtual TypeHandle get_type() const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
#endif
};

#endif

