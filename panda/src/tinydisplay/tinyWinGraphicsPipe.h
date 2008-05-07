// Filename: tinyWinGraphicsPipe.h
// Created by:  drose (06May08)
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

#ifndef TINYWINGRAPHICSPIPE_H
#define TINYWINGRAPHICSPIPE_H

#include "pandabase.h"

#ifdef WIN32

#include "winGraphicsPipe.h"
#include "tinyGraphicsStateGuardian.h"

////////////////////////////////////////////////////////////////////
//       Class : TinyWinGraphicsPipe
// Description : This graphics pipe represents the interface for
//               creating TinyGL graphics windows on a Windows-based
//               client.
////////////////////////////////////////////////////////////////////
class EXPCL_TINYDISPLAY TinyWinGraphicsPipe : public WinGraphicsPipe {
public:
  TinyWinGraphicsPipe();
  virtual ~TinyWinGraphicsPipe();

  virtual string get_interface_name() const;
  static PT(GraphicsPipe) pipe_constructor();

protected:
  virtual PT(GraphicsOutput) make_output(const string &name,
                                         const FrameBufferProperties &fb_prop,
                                         const WindowProperties &win_prop,
                                         int flags,
                                         GraphicsStateGuardian *gsg,
                                         GraphicsOutput *host,
                                         int retry,
                                         bool &precertify);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    WinGraphicsPipe::init_type();
    register_type(_type_handle, "TinyWinGraphicsPipe",
                  WinGraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "tinyWinGraphicsPipe.I"

#endif  // WIN32

#endif
