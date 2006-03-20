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

////////////////////////////////////////////////////////////////////
//       Class : OSXGraphicsBuffer
//  rhh mar-2006
//  Sorry ... this is not functional at all... I have no need for it yet ?
// 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAGL osxGraphicsBuffer : public GraphicsBuffer {
public:
  osxGraphicsBuffer(GraphicsPipe *pipe, 
                    const string &name,
                    int x_size, int y_size, int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host);
  virtual ~osxGraphicsBuffer();


  virtual bool begin_frame(FrameMode mode);
  virtual void end_frame(FrameMode mode);
  virtual void release_gsg();


protected:
  virtual void close_buffer();
  virtual bool open_buffer();

   
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
