// Filename: pandagl.cxx
// Created by:  drose (15May00)
// 
////////////////////////////////////////////////////////////////////

#include "pandagl.h"

#include <config_glgsg.h>

#ifdef HAVE_WGL
#include <config_wgldisplay.h>
#endif

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
#endif
}
