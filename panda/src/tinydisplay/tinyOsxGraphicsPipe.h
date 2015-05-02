// Filename: tinyOsxGraphicsPipe.h
// Created by:  drose (12May08)
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

#ifndef TINYOSXGRAPHICSPIPE_H
#define TINYOSXGRAPHICSPIPE_H

#include "pandabase.h"

#if defined(IS_OSX) && !defined(BUILD_IPHONE) && defined(HAVE_CARBON) && !__LP64__

// We have to include this early, before anyone includes
// netinet/tcp.h, which will define TCP_NODELAY and other symbols and
// confuse the Apple system headers.
#include <Carbon/Carbon.h>

#include "graphicsPipe.h"
#include "tinyGraphicsStateGuardian.h"

////////////////////////////////////////////////////////////////////
//       Class : TinyOsxGraphicsPipe
// Description : This graphics pipe represents the interface for
//               creating TinyPanda graphics windows on a Mac client.
////////////////////////////////////////////////////////////////////
class EXPCL_TINYDISPLAY TinyOsxGraphicsPipe : public GraphicsPipe {
public:
  TinyOsxGraphicsPipe();
  virtual ~TinyOsxGraphicsPipe();

  virtual string get_interface_name() const;
  static PT(GraphicsPipe) pipe_constructor();

  static CGImageRef create_cg_image(const PNMImage &pnm_image);

private:
  static void release_data(void *info, const void *data, size_t size);

protected:
  virtual PT(GraphicsOutput) make_output(const string &name,
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
    GraphicsPipe::init_type();
    register_type(_type_handle, "TinyOsxGraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "tinyOsxGraphicsPipe.I"

#endif  // IS_OSX

#endif
