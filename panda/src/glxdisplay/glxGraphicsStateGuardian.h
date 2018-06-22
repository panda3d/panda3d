/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glxGraphicsStateGuardian.h
 * @author drose
 * @date 2003-01-27
 */

#ifndef GLXGRAPHICSSTATEGUARDIAN_H
#define GLXGRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"

#include "glgsg.h"
#include "glxGraphicsPipe.h"
#include "posixGraphicsStateGuardian.h"

#if defined(GLX_VERSION_1_4)
// If the system header files give us version 1.4, we can assume it's safe to
// compile in a reference to glxGetProcAddress().
#define HAVE_GLXGETPROCADDRESS 1

#elif defined(GLX_ARB_get_proc_address)
// Maybe the system header files give us the corresponding ARB call.
#define HAVE_GLXGETPROCADDRESSARB 1

// Sometimes the system header files don't define this prototype for some
// reason.
extern "C" void (*glXGetProcAddressARB(const GLubyte *procName))( void );

#endif

// This must be included after we have included glgsg.h (which includes gl.h).
#include "panda_glxext.h"

// drose: the version of GLglx.h that ships with Fedora Core 2 seems to define
// GLX_VERSION_1_4, but for some reason does not define GLX_SAMPLE_BUFFERS or
// GLX_SAMPLES.  We work around that here.

#ifndef GLX_SAMPLE_BUFFERS
#define GLX_SAMPLE_BUFFERS                 100000
#endif
#ifndef GLX_SAMPLES
#define GLX_SAMPLES                        100001
#endif

// These typedefs are declared in glxext.h, but we must repeat them here,
// mainly because they will not be included from glxext.h if the system GLX
// version matches or exceeds the GLX version in which these functions are
// defined, and the system glx.h sometimes doesn't declare these typedefs.
#ifndef __EDG__  // Protect the following from the Tau instrumentor.
typedef __GLXextFuncPtr (* PFNGLXGETPROCADDRESSPROC) (const GLubyte *procName);
typedef int (* PFNGLXSWAPINTERVALSGIPROC) (int interval);

typedef GLXFBConfig * (* PFNGLXCHOOSEFBCONFIGPROC) (X11_Display *dpy, int screen, const int *attrib_list, int *nelements);
typedef GLXContext (* PFNGLXCREATENEWCONTEXTPROC) (X11_Display *dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct);
typedef XVisualInfo * (* PFNGLXGETVISUALFROMFBCONFIGPROC) (X11_Display *dpy, GLXFBConfig config);
typedef int (* PFNGLXGETFBCONFIGATTRIBPROC) (X11_Display *dpy, GLXFBConfig config, int attribute, int *value);
typedef GLXPixmap (* PFNGLXCREATEPIXMAPPROC) (X11_Display *dpy, GLXFBConfig config, Pixmap pixmap, const int *attrib_list);
typedef GLXPbuffer (* PFNGLXCREATEPBUFFERPROC) (X11_Display *dpy, GLXFBConfig config, const int *attrib_list);
typedef void (* PFNGLXDESTROYPBUFFERPROC) (X11_Display *dpy, GLXPbuffer pbuf);
typedef GLXContext ( *PFNGLXCREATECONTEXTATTRIBSARBPROC) (X11_Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list);
#endif  // __EDG__

/**
 * A tiny specialization on GLGraphicsStateGuardian to add some glx-specific
 * information.
 */
class glxGraphicsStateGuardian : public PosixGraphicsStateGuardian {
public:
  INLINE const FrameBufferProperties &get_fb_properties() const;
  void get_properties(FrameBufferProperties &properties, XVisualInfo *visual);
  void get_properties_advanced(FrameBufferProperties &properties,
                               bool &context_has_pbuffer, bool &pixmap_supported,
                               bool &slow, GLXFBConfig config);
  void choose_pixel_format(const FrameBufferProperties &properties,
                           X11_Display *_display,
                           int _screen,
                           bool need_pbuffer, bool need_pixmap);

  glxGraphicsStateGuardian(GraphicsEngine *engine, GraphicsPipe *pipe,
                           glxGraphicsStateGuardian *share_with);

  virtual ~glxGraphicsStateGuardian();

  bool glx_is_at_least_version(int major_version, int minor_version) const;

  GLXContext _share_context;
  GLXContext _context;
  X11_Display *_display;
  int _screen;
  XVisualInfo *_visual;
  XVisualInfo *_visuals;

  GLXFBConfig _fbconfig;
  FrameBufferProperties _fbprops;
  bool _context_has_pbuffer;  // true if the particular fbconfig supports pbuffers
  bool _context_has_pixmap;
  bool _slow;

public:
  bool _supports_swap_control;
  PFNGLXSWAPINTERVALSGIPROC _glXSwapIntervalSGI;

  bool _supports_fbconfig;
  PFNGLXCHOOSEFBCONFIGPROC _glXChooseFBConfig;
  PFNGLXCREATENEWCONTEXTPROC _glXCreateNewContext;
  PFNGLXGETVISUALFROMFBCONFIGPROC _glXGetVisualFromFBConfig;
  PFNGLXGETFBCONFIGATTRIBPROC _glXGetFBConfigAttrib;
  PFNGLXCREATEPIXMAPPROC _glXCreatePixmap;
  PFNGLXCREATECONTEXTATTRIBSARBPROC _glXCreateContextAttribs;

  bool _supports_pbuffer;  // true if the interface is available.
  bool _uses_sgix_pbuffer;
  PFNGLXCREATEPBUFFERPROC _glXCreatePbuffer;
  PFNGLXCREATEGLXPBUFFERSGIXPROC _glXCreateGLXPbufferSGIX;
  PFNGLXDESTROYPBUFFERPROC _glXDestroyPbuffer;

protected:
  virtual void gl_flush() const;
  virtual GLenum gl_get_error() const;

  virtual void query_gl_version();
  virtual void get_extra_extensions();
  virtual void *do_get_extension_func(const char *name);

private:
  void query_glx_extensions();
  void show_glx_client_string(const std::string &name, int id);
  void show_glx_server_string(const std::string &name, int id);
  void choose_temp_visual(const FrameBufferProperties &properties);
  void init_temp_context();
  void destroy_temp_xwindow();

  int _glx_version_major, _glx_version_minor;

  bool _checked_get_proc_address;
  PFNGLXGETPROCADDRESSPROC _glXGetProcAddress;

  GLXContext _temp_context;
  X11_Window _temp_xwindow;
  Colormap _temp_colormap;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PosixGraphicsStateGuardian::init_type();
    register_type(_type_handle, "glxGraphicsStateGuardian",
                  PosixGraphicsStateGuardian::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glxGraphicsStateGuardian.I"

#endif
