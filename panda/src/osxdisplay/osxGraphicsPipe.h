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

#ifndef OSXGRAPHICSPIPE_H
#define OSXGRAPHICSPIPE_H

#include "pandabase.h"
#include "graphicspipe.h"

#include <Carbon/Carbon.h>

class osxGraphicsStateGuardian;
class PNMImage;

////////////////////////////////////////////////////////////////////
//       Class : osxGraphicsPipe
// Description : This graphics pipe represents the interface for
//               creating OpenGL graphics windows on the various
//               OSX's.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAGL osxGraphicsPipe : public GraphicsPipe {
public:
  osxGraphicsPipe();
  virtual ~osxGraphicsPipe();

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
                                         GraphicsStateGuardian *gsg,
                                         GraphicsOutput *host,
                                         int retry,
                                         bool &precertify);

private:
 public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsPipe::init_type();
    register_type(_type_handle, "osxGraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class osxGraphicsBuffer;
};

#endif
