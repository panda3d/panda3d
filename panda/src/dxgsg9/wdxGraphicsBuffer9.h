/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wdxGraphicsBuffer9.h
 * @author drose
 * @date 2004-02-08
 */

#ifndef wdxGraphicsBuffer9_H
#define wdxGraphicsBuffer9_H

#include "pandabase.h"
#include "graphicsBuffer.h"
#include "dxgsg9base.h"
#include "dxTextureContext9.h"

/**
 * An offscreen render buffer.  In OpenGL under Windows, this simply renders
 * into a window that is never made visible.  There's a Windows interface for
 * rendering into a DIB, but this puts restrictions on the kind of pixelformat
 * we can use, and thus makes it difficult to support one GSG rendering into
 * an offscreen buffer and also into a window.
 */
class EXPCL_PANDADX wdxGraphicsBuffer9 : public GraphicsBuffer {
public:
  wdxGraphicsBuffer9(GraphicsEngine *engine, GraphicsPipe *pipe,
                     const std::string &name,
                     const FrameBufferProperties &fb_prop,
                     const WindowProperties &win_prop,
                     int flags,
                     GraphicsStateGuardian *gsg,
                     GraphicsOutput *host);
  virtual ~wdxGraphicsBuffer9();

  virtual INLINE bool get_supports_render_texture() const;

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

  virtual void select_target_tex_page(int page);

  virtual void process_events();

  virtual bool share_depth_buffer(GraphicsOutput *graphics_output);
  virtual void unshare_depth_buffer();

  void register_shared_depth_buffer(GraphicsOutput *graphics_output);
  void unregister_shared_depth_buffer(GraphicsOutput *graphics_output);

protected:
  virtual void close_buffer();
  virtual bool open_buffer();

private:
  bool save_bitplanes();
  bool rebuild_bitplanes();
  void restore_bitplanes();
  static void process_1_event();

  int _cube_map_index;
  DXGraphicsStateGuardian9 *_dxgsg;
  IDirect3DSurface9 *_saved_color_buffer;
  IDirect3DSurface9 *_saved_depth_buffer;
  D3DSURFACE_DESC    _saved_color_desc;
  D3DSURFACE_DESC    _saved_depth_desc;
  IDirect3DSurface9 *_color_backing_store;
  IDirect3DSurface9 *_depth_backing_store;
  int _backing_sizex;
  int _backing_sizey;

  wdxGraphicsBuffer9 *_shared_depth_buffer;
  std::list <wdxGraphicsBuffer9 *> _shared_depth_buffer_list;

  wdxGraphicsBuffer9 **_this;

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
  bool _debug;
  static TypeHandle _type_handle;

  friend class DXGraphicsStateGuardian9;
  friend class DXTextureContext9;
};

#include "wdxGraphicsBuffer9.I"

#endif
