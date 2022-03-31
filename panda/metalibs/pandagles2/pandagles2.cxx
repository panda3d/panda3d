/**
 * @file pandagles2.cxx
 * @author rdb
 * @date 2009-06-08
 */

#include "pandagles2.h"

#define OPENGLES_2
#include "config_gles2gsg.h"

#if defined(ANDROID)
#include "config_androiddisplay.h"
#include "androidGraphicsPipe.h"
#else
#include "config_egldisplay.h"
#include "eglGraphicsPipe.h"
#endif

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpandagles2() {
  init_libgles2gsg();

#if defined(ANDROID)
  init_libandroiddisplay();
#else
  init_libegldisplay();
#endif
}

/**
 * Returns the TypeHandle index of the recommended graphics pipe type defined
 * by this module.
 */
int
get_pipe_type_pandagles2() {
#if defined(ANDROID)
  return AndroidGraphicsPipe::get_class_type().get_index();
#else
  return eglGraphicsPipe::get_class_type().get_index();
#endif
}
