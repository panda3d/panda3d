// Filename: pandagles2.cxx
// Created by:  pro-rsoft (8Jun09)
//
////////////////////////////////////////////////////////////////////

#include "pandagles2.h"

#define OPENGLES_2
#include "config_gles2gsg.h"

#ifdef HAVE_EGL
#include "config_egldisplay.h"
#include "eglGraphicsPipe.h"
#endif

// By including checkPandaVersion.h, we guarantee that runtime
// attempts to load libpandagles2.so/.dll will fail if they inadvertently
// link with the wrong version of libdtool.so/.dll.

#include "checkPandaVersion.h"

////////////////////////////////////////////////////////////////////
//     Function: init_libpandagles2
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpandagles2() {
  init_libgles2gsg();

#ifdef HAVE_EGL
  init_libegldisplay();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: get_pipe_type_pandagles2
//  Description: Returns the TypeHandle index of the recommended
//               graphics pipe type defined by this module.
////////////////////////////////////////////////////////////////////
int
get_pipe_type_pandagles2() {
#ifdef HAVE_EGL
  return eglGraphicsPipe::get_class_type().get_index();
#endif

  return 0;
}
