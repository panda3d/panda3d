// Filename: dxGraphicsDevice.h
// Created by:  masad (02Jan04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2004, Disney Enterprises, Inc.  All rights reserved
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
  LPDIRECT3DDEVICE9 _pD3DDevice;  // same as pScrn->_pD3DDevice, cached for spd
  IDirect3DSwapChain9 *_pSwapChain;

#if 0
protected:
  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
#endif
};

#endif

