// Filename: wglGraphicsBuffer.h
// Created by:  drose (08Feb04)
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

#ifndef WGLGRAPHICSBUFFER_H
#define WGLGRAPHICSBUFFER_H

#include "pandabase.h"
#include "graphicsBuffer.h"
#include "glgsg.h"

// This must be included after we have included glgsg.h (which
// includes gl.h).
#include "wglext.h"

#include <windows.h>

////////////////////////////////////////////////////////////////////
//       Class : wglGraphicsBuffer
// Description : An offscreen render buffer.  In OpenGL under Windows,
//               this simply renders into a window that is never made
//               visible.  There's a Windows interface for rendering
//               into a DIB, but this puts restrictions on the kind of
//               pixelformat we can use, and thus makes it difficult
//               to support one GSG rendering into an offscreen buffer
//               and also into a window.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAGL wglGraphicsBuffer : public GraphicsBuffer {
public:
  wglGraphicsBuffer(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                    const string &name,
                    int x_size, int y_size);
  virtual ~wglGraphicsBuffer();

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
  bool make_pbuffer(HDC window_dc);
  int choose_pbuffer_format(HDC twindow_dc, bool draw_to_texture);

  static void process_1_event();

  HPBUFFERARB _pbuffer;
  HDC _pbuffer_dc;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsBuffer::init_type();
    register_type(_type_handle, "wglGraphicsBuffer",
                  GraphicsBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class wglGraphicsStateGuardian;
};

#include "wglGraphicsBuffer.I"

#endif
