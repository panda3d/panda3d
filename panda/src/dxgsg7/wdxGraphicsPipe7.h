// Filename: wdxGraphicsPipe7.h
// Created by:  drose (20Dec02)
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

#ifndef WDXGRAPHICSPIPE7_H
#define WDXGRAPHICSPIPE7_H

#include "pandabase.h"
#include "winGraphicsPipe.h"

////////////////////////////////////////////////////////////////////
//       Class : wdxGraphicsPipe7
// Description : This graphics pipe represents the interface for
//               creating DirectX graphics windows.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX wdxGraphicsPipe7 : public WinGraphicsPipe {
public:
  wdxGraphicsPipe7();
  virtual ~wdxGraphicsPipe7();

  virtual string get_interface_name() const;
  static PT(GraphicsPipe) pipe_constructor();
  virtual PT(GraphicsStateGuardian) make_gsg(const FrameBufferProperties &properties);

protected:
  virtual PT(GraphicsWindow) make_window(GraphicsStateGuardian *gsg, const string &name);

private:
  bool init();

private:
  HINSTANCE _hDDrawDLL;

  typedef HRESULT (WINAPI * LPDIRECTDRAWCREATEEX)(GUID FAR * lpGuid, LPVOID *lplpDD, REFIID iid, IUnknown FAR *pUnkOuter);
  LPDIRECTDRAWCREATEEX _DirectDrawCreateEx;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    WinGraphicsPipe::init_type();
    register_type(_type_handle, "wdxGraphicsPipe7",
                  WinGraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}


private:
  static TypeHandle _type_handle;

  friend class wdxGraphicsWindow7;
};

#include "wdxGraphicsPipe7.I"

#endif
