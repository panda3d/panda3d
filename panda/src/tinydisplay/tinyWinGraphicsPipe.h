/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tinyWinGraphicsPipe.h
 * @author drose
 * @date 2008-05-06
 */

#ifndef TINYWINGRAPHICSPIPE_H
#define TINYWINGRAPHICSPIPE_H

#include "pandabase.h"

#ifdef WIN32

#include "winGraphicsPipe.h"
#include "tinyGraphicsStateGuardian.h"

/**
 * This graphics pipe represents the interface for creating TinyPanda graphics
 * windows on a Windows-based client.
 */
class EXPCL_TINYDISPLAY TinyWinGraphicsPipe : public WinGraphicsPipe {
public:
  TinyWinGraphicsPipe();
  virtual ~TinyWinGraphicsPipe();

  virtual std::string get_interface_name() const;
  static PT(GraphicsPipe) pipe_constructor();

protected:
  virtual PT(GraphicsOutput) make_output(const std::string &name,
                                         const FrameBufferProperties &fb_prop,
                                         const WindowProperties &win_prop,
                                         int flags,
                                         GraphicsEngine *engine,
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
