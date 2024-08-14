/**
 * @file pandagl.cxx
 * @author drose
 * @date 2000-05-15
 */

#include "pandagl.h"

#include "config_glgsg.h"

#ifdef HAVE_WGL
#include "config_wgldisplay.h"
#include "wglGraphicsPipe.h"
#endif

#ifdef HAVE_COCOA
#include "config_cocoagldisplay.h"
#include "cocoaGLGraphicsPipe.h"
#endif

#ifdef HAVE_GLX
#include "config_glxdisplay.h"
#include "glxGraphicsPipe.h"
#endif

#ifdef HAVE_EGL
#include "config_egldisplay.h"
#include "eglGraphicsPipe.h"
#endif

#if !defined(HAVE_WGL) && !defined(HAVE_COCOA) && !defined(HAVE_GLX) && !defined(HAVE_EGL)
#error One of HAVE_WGL, HAVE_COCOA, HAVE_GLX or HAVE_EGL must be defined when compiling pandagl!
#endif

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpandagl() {
  init_libglgsg();

#ifdef HAVE_WGL
  init_libwgldisplay();
#endif  // HAVE_GL

#ifdef HAVE_COCOA
  init_libcocoagldisplay();
#endif

#ifdef HAVE_GLX
  init_libglxdisplay();
#endif

#ifdef HAVE_EGL
  init_libegldisplay();
#endif
}

/**
 * Returns the TypeHandle index of the recommended graphics pipe type defined
 * by this module.
 */
int
get_pipe_type_pandagl() {
#if defined(HAVE_WGL)
  return wglGraphicsPipe::get_class_type().get_index();

#elif defined(HAVE_COCOA)
  return CocoaGLGraphicsPipe::get_class_type().get_index();

#elif defined(HAVE_GLX)
  return glxGraphicsPipe::get_class_type().get_index();

#elif defined(HAVE_EGL)
  return eglGraphicsPipe::get_class_type().get_index();

#else
  return 0;
#endif
}

#if defined(HAVE_EGL) && !defined(USE_X11)
int
get_pipe_type_p3headlessgl() {
  return eglGraphicsPipe::get_class_type().get_index();
}
#endif
