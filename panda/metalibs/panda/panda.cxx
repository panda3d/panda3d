// Filename: panda.cxx
// Created by:  drose (15May00)
// 
////////////////////////////////////////////////////////////////////

#include "panda.h"

#include "config_device.h"
#include "config_display.h"
#include "config_pnmimagetypes.h"
#include "config_pgraph.h"
#ifdef DO_PSTATS
#include "config_pstats.h"
#endif

#ifdef LINK_IN_GL
#include "config_glgsg.h"
#ifdef HAVE_WGL
#include "config_wgldisplay.h"
#endif
#endif

#ifdef LINK_IN_PHYSICS
#include "config_physics.h"
#include "config_particlesystem.h"
#endif

// By including checkPandaVersion.h, we guarantee that runtime
// attempts to load libpanda.so/.dll will fail if they inadvertently
// link with the wrong version of libdtool.so/.dll.

#include "checkPandaVersion.h"

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
  init_libpgraph();
  init_libpnmimagetypes();
#ifdef DO_PSTATS
  init_libpstatclient();
#endif

#ifdef LINK_IN_GL
  init_libglgsg();
#ifdef HAVE_WGL
  init_libwgldisplay();
#endif
#endif

#ifdef LINK_IN_PHYSICS
  init_libphysics();
  init_libparticlesystem();
#endif
}
