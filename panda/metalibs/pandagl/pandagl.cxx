// Filename: pandagl.cxx
// Created by:  drose (15May00)
// 
////////////////////////////////////////////////////////////////////

#include "pandagl.h"

#include "config_glgsg.h"

#ifdef HAVE_WGL
#include "config_wgldisplay.h"
#include "wglGraphicsPipe.h"
#endif

#ifdef IS_OSX
#include "config_osxdisplay.h"
#include "osxGraphicsPipe.h"
#endif

#ifdef IS_LINUX
#include "config_glxdisplay.h"
#include "glxGraphicsPipe.h"
#endif

// By including checkPandaVersion.h, we guarantee that runtime
// attempts to load libpandagl.so/.dll will fail if they inadvertently
// link with the wrong version of libdtool.so/.dll.

#include "checkPandaVersion.h"

////////////////////////////////////////////////////////////////////
//     Function: init_libpandagl
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpandagl() {
  init_libglgsg();

#ifdef HAVE_WGL
  init_libwgldisplay();
#endif  // HAVE_GL

#ifdef IS_OSX
  init_libosxdisplay();
#endif

#ifdef IS_LINUX
  init_libglxdisplay();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: get_pipe_type_pandagl
//  Description: Returns the TypeHandle index of the recommended
//               graphics pipe type defined by this module.
////////////////////////////////////////////////////////////////////
int
get_pipe_type_pandagl() {
#ifdef HAVE_WGL
  return wglGraphicsPipe::get_class_type().get_index();
#endif

#ifdef IS_OSX
  return osxGraphicsPipe::get_class_type().get_index();
#endif

#ifdef IS_LINUX
  return glxGraphicsPipe::get_class_type().get_index();
#endif

  return 0;
}
