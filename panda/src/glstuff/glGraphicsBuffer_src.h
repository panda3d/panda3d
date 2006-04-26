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
//               This design has significant advantages over the
//               older wglGraphicsBuffer and glxGraphicsBuffer:
//
//               * Can export depth and stencil.
//               * Supports auxiliary bitplanes.
//               * Supports non-power-of-two padding.
//               * Supports tracking of host window size.
//               * Supports cumulative render-to-texture.
//               * Faster than pbuffers.
//               * Can render onto a texture without clearing it first.
//
//               Some of these deserve a little explanation. 
//               Auxiliary bitplanes are additional bitplanes above
//               and beyond the normal depth,stencil,color.  One can
//               use them to render out multiple textures in a single
//               pass.  Cumulative render-to-texture means that if
//               don't clear the buffer, then the contents of the
//               buffer will be equal to the texture's previous
//               contents.  This alo means you can meaningfully
//               share a bitplane between two buffers by binding
//               the same texture to both buffers. 
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

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

  virtual void select_cube_map(int cube_map_index);

protected:
  virtual void close_buffer();
  virtual bool open_buffer();

private:
  
  void bind_slot(bool rb_resize, Texture **attach, RenderTexturePlane plane,
                 GLenum attachpoint, GLenum texformat, Texture::Format fmt);
  bool check_fbo();
  void generate_mipmaps();
  void rebuild_bitplanes();
  
  GLuint      _fbo;
  int         _rb_size_x;
  int         _rb_size_y;
  int         _cube_face_active;
  PT(Texture) _tex[RTP_COUNT];
  GLuint      _rb[RTP_COUNT];
  GLenum      _attach_point[RTP_COUNT];
  
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

