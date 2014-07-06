// Filename: glGraphicsBuffer_src.h
// Created by:  jyelon (15Jan06)
// Modified by: kleonard (27Jun07)
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
//               * Supports multisample antialiased rendering.
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
//               This class now also uses the extensions 
//               EXT_framebuffer_multisample and EXT_framebuffer_blit
//               to allow for multisample antialiasing these offscreen
//               render targets.  If these extensions are unavailable
//               the buffer will render as if multisamples is 0.
//
////////////////////////////////////////////////////////////////////

class EXPCL_GL CLP(GraphicsBuffer) : public GraphicsBuffer {
public:
  CLP(GraphicsBuffer)(GraphicsEngine *engine, GraphicsPipe *pipe,
                      const string &name,
                      const FrameBufferProperties &fb_prop,
                      const WindowProperties &win_prop,
                      int flags,
                      GraphicsStateGuardian *gsg,
                      GraphicsOutput *host);
  virtual ~CLP(GraphicsBuffer)();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

  virtual void set_size(int x, int y);

  virtual void select_target_tex_page(int page);

  virtual bool share_depth_buffer(GraphicsOutput *graphics_output);
  virtual void unshare_depth_buffer();

  virtual bool get_supports_render_texture() const;

  void register_shared_depth_buffer(GraphicsOutput *graphics_output);
  void unregister_shared_depth_buffer(GraphicsOutput *graphics_output);

protected:
  virtual void close_buffer();
  virtual bool open_buffer();
  
  void check_host_valid();
  
  void report_my_errors(int line, const char *file);

private:
  
  void bind_slot(int layer, bool rb_resize, Texture **attach,
                 RenderTexturePlane plane, GLenum attachpoint);
  void bind_slot_multisample(bool rb_resize, Texture **attach,
                 RenderTexturePlane plane, GLenum attachpoint);
  void attach_tex(int layer, int view, Texture *attach, GLenum attachpoint);
  bool check_fbo();
  void generate_mipmaps();
  void rebuild_bitplanes();
  void resolve_multisamples();

  // We create one FBO for each cube map face we'll be rendering to.
  // If we aren't rendering to any cube maps, we use only _fbo[0].
  pvector<GLuint> _fbo;

  // For multisample we render first to a multisample buffer, then
  // filter it to _fbo[face] at the end of the frame.
  GLuint      _fbo_multisample;
  int         _requested_multisamples;
  int         _requested_coverage_samples;
  bool        _use_depth_stencil;
  bool        _have_any_color;

  int         _rb_size_x;
  int         _rb_size_y;
  int         _rb_size_z;

  // The texture or render buffer bound to each plane.
  PT(Texture) _tex[RTP_COUNT];
  GLuint      _rb[RTP_COUNT];

  // The render buffer for _fbo_multisample.
  GLuint      _rbm[RTP_COUNT];

  // The cube map face we are currently drawing to or have just
  // finished drawing to, or -1 if we are not drawing to a cube map.
  int _bound_tex_page;

  bool _initial_clear;
  bool _needs_rebuild;
  UpdateSeq _last_textures_seq;
  
  CLP(GraphicsBuffer) *_shared_depth_buffer;
  list <CLP(GraphicsBuffer) *> _shared_depth_buffer_list;
  
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

  INLINE int get_multisample_count();
  INLINE int get_coverage_sample_count();

private:
  static TypeHandle _type_handle;

  friend class CLP(GraphicsStateGuardian);
};

#include "glGraphicsBuffer_src.I"
