// Filename: panda.cxx
// Created by:  drose (15May00)
// 
////////////////////////////////////////////////////////////////////

#include "panda.h"

#include <config_device.h>
#include <config_display.h>
#include <config_graph.h>
#include <config_pnmimagetypes.h>
#include <config_pstats.h>
#include <config_sgraph.h>

////////////////////////////////////////////////////////////////////
//     Function: init_libpanda
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpanda() {
  init_libdevice();
  init_libdisplay();
  init_libgraph();
  init_libpnmimagetypes();
  init_libpstatclient();
  init_libsgraph();
}
