// Filename: pandacr.cxx
// Created by:  skyler, baseed on pandagl
// 
////////////////////////////////////////////////////////////////////

#include "pandacr.h"

#include <config_crgsg.h>
#ifdef HAVE_WCR
#include <config_wcrdisplay.h>
#endif

////////////////////////////////////////////////////////////////////
//     Function: init_libpandacr
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpandacr() {
  init_libcrgsg();
#ifdef HAVE_WCR
  init_libwcrdisplay();
#endif
}
