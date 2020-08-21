/**
 * @file pandagles.cxx
 * @author rdb
 * @date 2009-06-08
 */

#include "pandagles.h"

#define OPENGLES_1
#include "config_glesgsg.h"

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
init_libpandagles() {
  init_libglesgsg();

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
get_pipe_type_pandagles() {
#if defined(ANDROID)
  return AndroidGraphicsPipe::get_class_type().get_index();
#else
  return eglGraphicsPipe::get_class_type().get_index();
#endif
}
