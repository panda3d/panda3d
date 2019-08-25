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

#if defined(HAVE_COCOA)
#include "config_cocoadisplay.h"
#include "cocoaGraphicsPipe.h"
#endif

#ifdef HAVE_GLX
#include "config_glxdisplay.h"
#include "glxGraphicsPipe.h"
#endif

#if !defined(HAVE_WGL) && !defined(HAVE_COCOA) && !defined(HAVE_GLX)
#error One of HAVE_WGL, HAVE_COCOA or HAVE_GLX must be defined when compiling pandagl!
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

#if defined(HAVE_COCOA)
  init_libcocoadisplay();
#endif

#ifdef IS_LINUX
  init_libglxdisplay();
#endif
}

/**
 * Returns the TypeHandle index of the recommended graphics pipe type defined
 * by this module.
 */
int
get_pipe_type_pandagl() {
#ifdef HAVE_WGL
  return wglGraphicsPipe::get_class_type().get_index();
#endif

#if defined(HAVE_COCOA)
  return CocoaGraphicsPipe::get_class_type().get_index();
#endif

#ifdef HAVE_GLX
  return glxGraphicsPipe::get_class_type().get_index();
#endif

  return 0;
}
