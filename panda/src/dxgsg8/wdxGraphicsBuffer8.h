// Filename: wdxGraphicsBuffer8.h
// Created by:  drose (08Feb04)
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

#ifndef wdxGraphicsBuffer8_H
#define wdxGraphicsBuffer8_H

#include "pandabase.h"
#include "graphicsBuffer.h"
#include "dxgsg8base.h"


////////////////////////////////////////////////////////////////////
//       Class : wdxGraphicsBuffer8
// Description : An offscreen render buffer.  In OpenGL under Windows,
//               this simply renders into a window that is never made
//               visible.  There's a Windows interface for rendering
//               into a DIB, but this puts restrictions on the kind of
//               pixelformat we can use, and thus makes it difficult
//               to support one GSG rendering into an offscreen buffer
//               and also into a window.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX wdxGraphicsBuffer8 : public GraphicsBuffer {
public:
  wdxGraphicsBuffer8(GraphicsEngine *engine, GraphicsPipe *pipe,
                     const string &name,
                     const FrameBufferProperties &fb_prop,
                     const WindowProperties &win_prop,
                     int flags,
                     GraphicsStateGuardian *gsg,
                     GraphicsOutput *host);
  virtual ~wdxGraphicsBuffer8();
  
  virtual INLINE bool get_supports_render_texture() const;

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

  virtual void select_target_tex_page(int page);

  virtual void process_events();

protected:
  virtual void close_buffer();
  virtual bool open_buffer();

private:
  bool save_bitplanes();
  bool rebuild_bitplanes();
  void restore_bitplanes();
  static void process_1_event();

  int _cube_map_index;
  DXGraphicsStateGuardian8 *_dxgsg;
  IDirect3DSurface8 *_saved_color_buffer;
  IDirect3DSurface8 *_saved_depth_buffer;
  D3DSURFACE_DESC    _saved_color_desc;
  D3DSURFACE_DESC    _saved_depth_desc;
  IDirect3DSurface8 *_color_backing_store;
  IDirect3DSurface8 *_depth_backing_store;
  int _backing_sizex;
  int _backing_sizey;

  wdxGraphicsBuffer8 **_this;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsBuffer::init_type();
    register_type(_type_handle, "wdxGraphicsBuffer8",
                  GraphicsBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class DXGraphicsStateGuardian8;
  friend class DXTextureContext8;
};

#include "wdxGraphicsBuffer8.I"

#endif
