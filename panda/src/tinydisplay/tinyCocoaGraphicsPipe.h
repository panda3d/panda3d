/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tinyCocoaGraphicsPipe.h
 * @author rdb
 * @date 2023-03-21
 */

#ifndef TINYCOCOAGRAPHICSPIPE_H
#define TINYCOCOAGRAPHICSPIPE_H

#include "pandabase.h"

#ifdef HAVE_COCOA

#include "cocoaGraphicsWindow.h"
#include "cocoaGraphicsPipe.h"
#include "tinyGraphicsStateGuardian.h"

/**
 * This graphics pipe represents the interface for creating TinyPanda graphics
 * windows on a Cocoa-based (macOS) client.
 */
class EXPCL_TINYDISPLAY TinyCocoaGraphicsPipe : public CocoaGraphicsPipe {
public:
  TinyCocoaGraphicsPipe(CGDirectDisplayID display = CGMainDisplayID());
  virtual ~TinyCocoaGraphicsPipe();

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
    CocoaGraphicsPipe::init_type();
    register_type(_type_handle, "TinyCocoaGraphicsPipe",
                  CocoaGraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "tinyCocoaGraphicsPipe.I"

#endif  // HAVE_COCOA

#endif
