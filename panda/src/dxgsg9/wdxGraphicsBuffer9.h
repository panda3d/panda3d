// Filename: wdxGraphicsBuffer9.h
// Created by:  drose (08Feb04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2005, Disney Enterprises, Inc.  All rights reserved
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

#ifndef wdxGraphicsBuffer9_H
#define wdxGraphicsBuffer9_H

#include "pandabase.h"
#include "graphicsBuffer.h"
#include "dxgsg9base.h"
#include "dxTextureContext9.h"

////////////////////////////////////////////////////////////////////
//       Class : wdxGraphicsBuffer9
// Description : An offscreen render buffer.  In OpenGL under Windows,
//               this simply renders into a window that is never made
//               visible.  There's a Windows interface for rendering
//               into a DIB, but this puts restrictions on the kind of
//               pixelformat we can use, and thus makes it difficult
//               to support one GSG rendering into an offscreen buffer
//               and also into a window.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX wdxGraphicsBuffer9 : public GraphicsBuffer {
public:
  wdxGraphicsBuffer9(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                    const string &name,
                    int x_size, int y_size);
  virtual ~wdxGraphicsBuffer9();

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
  static void process_1_event();

  int _cube_map_index;
  IDirect3DSurface9 *_back_buffer;
  IDirect3DSurface9 *_z_stencil_buffer;
  IDirect3DSurface9 *_direct_3d_surface;
  DXTextureContext9 *_dx_texture_context9;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsBuffer::init_type();
    register_type(_type_handle, "wdxGraphicsBuffer9",
                  GraphicsBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class DXGraphicsStateGuardian9;
  friend class DXTextureContext9;
};

// #include "wdxGraphicsBuffer9.I"

#endif
