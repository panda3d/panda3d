// Filename: graphicsBuffer.h
// Created by:  mike (09Jan97)
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

#ifndef GRAPHICSBUFFER_H
#define GRAPHICSBUFFER_H

#include "pandabase.h"

#include "graphicsOutput.h"

////////////////////////////////////////////////////////////////////
//       Class : GraphicsBuffer
// Description : An offscreen buffer for rendering into.  Pretty much
//               all you can do with this is render into it and then
//               get the framebuffer out as an image.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsBuffer : public GraphicsOutput {
protected:
  GraphicsBuffer(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                 int x_size, int y_size);

PUBLISHED:
  virtual ~GraphicsBuffer();
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsOutput::init_type();
    register_type(_type_handle, "GraphicsBuffer",
                  GraphicsOutput::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "graphicsBuffer.I"

#endif /* GRAPHICSBUFFER_H */
