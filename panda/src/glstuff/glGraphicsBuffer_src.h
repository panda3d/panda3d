// Filename: glGraphicsBuffer_src.h
// Created by:  jyelon (15Jan06)
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

#include "pandabase.h"
#include "graphicsBuffer.h"

////////////////////////////////////////////////////////////////////
//       Class : glGraphicsBuffer
// Description : An offscreen render buffer.
//
//               The glGraphicsBuffer is based on the OpenGL
//               EXT_framebuffer_object and ARB_draw_buffers extensions.
//               This design has three significant advantages over the
//               wglGraphicsBuffer and glxGraphicsBuffer.
//
//               As you might expect, this type of buffer can export
//               its color buffer as a texture.  But it can also export
//               its depth buffer, its stencil buffer, and any number
//               of auxiliary buffers.  This is the biggest advantage:
//               it can render to many textures at the same time.
//
//               There is also a speed advantage.  When using a
//               glGraphicsBuffer, it is not necessary to call the
//               extremely expensive wglMakeCurrent on buffer switches.
//
//               The glGraphicsBuffer can also track the size of a host
//               window, and automatically resize itself to match.
//
//               If either of the necessary OpenGL extensions is not
//               available, then the glGraphicsBuffer will not be
//               available (although it may still be possible to
//               create a wglGraphicsBuffer or glxGraphicsBuffer).
//
////////////////////////////////////////////////////////////////////

class EXPCL_GL CLP(GraphicsBuffer) : public GraphicsBuffer {
public:
  CLP(GraphicsBuffer)(GraphicsPipe *pipe,
                      const string &name,
                      const FrameBufferProperties &properties,
                      int x_size, int y_size, int flags,
                      GraphicsStateGuardian *gsg,
                      GraphicsOutput *host);
  virtual ~CLP(GraphicsBuffer)();

  virtual bool begin_frame(FrameMode mode);
  virtual void end_frame(FrameMode mode);

  virtual void select_cube_map(int cube_map_index);

protected:
  virtual void close_buffer();
  virtual bool open_buffer();

private:
  
  void generate_mipmaps();
  void rebuild_bitplanes();
  
  enum {
    SLOT_color,
    SLOT_depth,
    SLOT_stencil,
    SLOT_COUNT
  };

  GLuint      _fbo;
  int         _rb_size_x;
  int         _rb_size_y;
  GLuint      _rb[SLOT_COUNT];
  PT(Texture) _tex[SLOT_COUNT];
  GLenum      _attach_point[SLOT_COUNT];
  GLenum      _slot_format[SLOT_COUNT];

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsBuffer::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "GraphicsBuffer",
                  GraphicsBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class CLP(GraphicsStateGuardian);
};

#include "glGraphicsBuffer_src.I"

