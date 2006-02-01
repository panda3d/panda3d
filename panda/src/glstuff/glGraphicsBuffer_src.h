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
#include "glgsg.h"

////////////////////////////////////////////////////////////////////
//       Class : glGraphicsBuffer
// Description : An offscreen render buffer.  This is implemented
//               using the following OpenGL extensions:
//               
//               EXT_framebuffer_object
//               ARB_draw_buffers
//               ARB_texture_non_power_of_two
//
//               If any of these extensions is missing, then
//               glGraphicsBuffer is not available (although it may
//               still be possible to create a wglGraphicsBuffer or
//               glxGraphicsBuffer).
//
//               The glGraphicsBuffer can export its color buffer as a
//               texture.  It can also export its depth buffer as a depth
//               texture and its stencil buffer as a stencil texture.
//               Finally, it can have auxiliary buffers (additional 
//               bitplanes above and beyond the usual set), which can
//               also be exported as textures.
//
////////////////////////////////////////////////////////////////////

class EXPCL_GL CLP(GraphicsBuffer) : public GraphicsBuffer {
public:
  CLP(GraphicsBuffer)(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                      const string &name,
                      int x_size, int y_size);
  virtual ~CLP(GraphicsBuffer)();

  virtual bool begin_frame();
  virtual void select_cube_map(int cube_map_index);

  virtual void make_current();
  virtual void release_gsg();

  virtual void begin_render_texture();
  virtual void end_render_texture();
  
  virtual void process_events();

protected:
  virtual void close_buffer();
  virtual bool open_buffer();

private:
  // HPBUFFERARB _pbuffer;
  // HDC _pbuffer_dc;

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

