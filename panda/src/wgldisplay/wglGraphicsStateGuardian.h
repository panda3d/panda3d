/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wglGraphicsStateGuardian.h
 * @author drose
 * @date 2003-01-27
 */

#ifndef WGLGRAPHICSSTATEGUARDIAN_H
#define WGLGRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"

#include "glgsg.h"

// This must be included after we have included glgsg.h (which includes gl.h).
#include "wglext.h"

/**
 * A tiny specialization on GLGraphicsStateGuardian to add some wgl-specific
 * information.
 */
class wglGraphicsStateGuardian : public GLGraphicsStateGuardian {
public:
  wglGraphicsStateGuardian(GraphicsEngine *engine, GraphicsPipe *pipe,
                           wglGraphicsStateGuardian *share_with);
  virtual ~wglGraphicsStateGuardian();

  INLINE int get_pfnum() const;
  INLINE bool pfnum_supports_pbuffer() const;
  INLINE const FrameBufferProperties &get_fb_properties() const;
  bool fail_pfnum();

  INLINE bool made_context() const;
  INLINE HGLRC get_context(HDC hdc);
  void get_properties(FrameBufferProperties &properties, HDC hdc, int pfnum);
  bool get_properties_advanced(FrameBufferProperties &properties, HDC hdc, int pfnum);
  void choose_pixel_format(const FrameBufferProperties &properties, bool need_pbuffer);
  virtual void reset();

  INLINE HDC get_twindow_dc();
  INLINE bool get_supports_wgl_render_texture() const;

  static bool get_gamma_table(void);
  static bool static_set_gamma(bool restore, PN_stdfloat gamma);
  bool set_gamma(PN_stdfloat gamma);
  void restore_gamma();
  static void atexit_function(void);


protected:
  virtual void get_extra_extensions();
  virtual void *do_get_extension_func(const char *name);

private:
  void make_context(HDC hdc);
  HGLRC get_share_context() const;
  void redirect_share_pool(wglGraphicsStateGuardian *share_with);


  bool make_twindow();
  void release_twindow();

  static void register_twindow_class();

  // We have to save a pointer to the GSG we intend to share texture context
  // with, since we don't create our own context in the constructor.
  PT(wglGraphicsStateGuardian) _share_with;

  // These properties are for all wglGraphicsWindow that use this gsg.
  FrameBufferProperties _pfnum_properties;
  bool _pfnum_supports_pbuffer;
  int _pfnum;

  // This pfnum is the pfnum chosen via DescribePixelFormat.  It is used in
  // case the one returned by wglChoosePixelFormatARB() fails.
  FrameBufferProperties _pre_pfnum_properties;
  int _pre_pfnum;

  bool _made_context;
  HGLRC _context;

  HWND _twindow;
  HDC _twindow_dc;

  static const char * const _twindow_class_name;
  static bool _twindow_class_registered;

public:
  bool _supports_swap_control;
  PFNWGLSWAPINTERVALEXTPROC _wglSwapIntervalEXT;

  bool _supports_pbuffer;
  PFNWGLCREATEPBUFFERARBPROC _wglCreatePbufferARB;
  PFNWGLGETPBUFFERDCARBPROC _wglGetPbufferDCARB;
  PFNWGLRELEASEPBUFFERDCARBPROC _wglReleasePbufferDCARB;
  PFNWGLDESTROYPBUFFERARBPROC _wglDestroyPbufferARB;
  PFNWGLQUERYPBUFFERARBPROC _wglQueryPbufferARB;

  bool _supports_pixel_format;
  PFNWGLGETPIXELFORMATATTRIBIVARBPROC _wglGetPixelFormatAttribivARB;
  PFNWGLGETPIXELFORMATATTRIBFVARBPROC _wglGetPixelFormatAttribfvARB;
  PFNWGLCHOOSEPIXELFORMATARBPROC _wglChoosePixelFormatARB;

  bool _supports_wgl_multisample;

  bool _supports_wgl_render_texture;
  PFNWGLBINDTEXIMAGEARBPROC _wglBindTexImageARB;
  PFNWGLRELEASETEXIMAGEARBPROC _wglReleaseTexImageARB;
  PFNWGLSETPBUFFERATTRIBARBPROC _wglSetPbufferAttribARB;

  PFNWGLCREATECONTEXTATTRIBSARBPROC _wglCreateContextAttribsARB;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GLGraphicsStateGuardian::init_type();
    register_type(_type_handle, "wglGraphicsStateGuardian",
                  GLGraphicsStateGuardian::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class wglGraphicsBuffer;
};

#include "wglGraphicsStateGuardian.I"

#endif
