// Filename: pandagl.cxx
// Created by:  drose (15May00)
// 
////////////////////////////////////////////////////////////////////

#include "pandagl.h"

#ifndef LINK_IN_GL
#include <config_glgsg.h>
// Temporarily commented out for development on wgldisplay
/*
#ifdef HAVE_WGL
#include <config_wgldisplay.h>
#endif
*/
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
#ifndef LINK_IN_GL
  init_libglgsg();
// Temporarily commented out for development on wgldisplay
  /*
#ifdef HAVE_WGL
  init_libwgldisplay();
#endif
  */
#endif
}
