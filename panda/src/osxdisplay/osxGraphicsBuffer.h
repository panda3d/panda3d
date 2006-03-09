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

#ifndef OSXGRAPHICSBUFFER_H
#define OSXGRAPHICSBUFFER_H
#include <Carbon/Carbon.h>

#define __glext_h_
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <AGL/agl.h>

#include "pandabase.h"
#include "graphicsBuffer.h"
#include "glgsg.h"

// This must be included after we have included glgsg.h (which
// includes gl.h).
//#include "wglext.h"

//#include <windows.h>

////////////////////////////////////////////////////////////////////
//       Class : OSXGraphicsBuffer
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAGL osxGraphicsBuffer : public GraphicsBuffer {
public:
  osxGraphicsBuffer(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                    const string &name,
                    int x_size, int y_size);
  virtual ~osxGraphicsBuffer();

  virtual bool begin_frame(FrameMode mode);
  virtual void end_frame(FrameMode mode);

  virtual void release_gsg();

//  virtual void begin_render_texture();
//  virtual void end_render_texture();
  

protected:
  virtual void close_buffer();
  virtual bool open_buffer();

private:
   
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsBuffer::init_type();
    register_type(_type_handle, "owsGraphicsBuffer",
                  GraphicsBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class osxGraphicsStateGuardian;
};


#endif
