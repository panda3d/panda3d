// Filename: wglGraphicsPipe.h
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

#ifndef WGLGRAPHICSPIPE_H
#define WGLGRAPHICSPIPE_H

#include "pandabase.h"
#include "winGraphicsPipe.h"

////////////////////////////////////////////////////////////////////
//       Class : wglGraphicsPipe
// Description : This graphics pipe represents the interface for
//               creating OpenGL graphics windows on the various
//               Windows OSes.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAGL wglGraphicsPipe : public WinGraphicsPipe {
public:
  wglGraphicsPipe();
  virtual ~wglGraphicsPipe();

  virtual string get_interface_name() const;
  static PT(GraphicsPipe) pipe_constructor();

protected:
  virtual PT(GraphicsStateGuardian) make_gsg(const FrameBufferProperties &properties);
  virtual PT(GraphicsWindow) make_window(GraphicsStateGuardian *gsg);

private:
  int choose_pfnum(FrameBufferProperties &properties, HDC hdc) const;
  int find_pixfmtnum(FrameBufferProperties &properties, HDC hdc,
                     bool bLookforHW) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    WinGraphicsPipe::init_type();
    register_type(_type_handle, "wglGraphicsPipe",
                  WinGraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "wglGraphicsPipe.I"

#endif
