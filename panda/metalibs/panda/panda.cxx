/**
 * @file panda.cxx
 * @author drose
 * @date 2000-05-15
 */

#include "panda.h"

#include "config_pnmimagetypes.h"
#include "config_device.h"
#include "config_display.h"
#include "config_pgraph.h"
#ifdef DO_PSTATS
#include "config_pstatclient.h"
#endif

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_LIBPANDA)
  #error Buildsystem error: BUILDING_LIBPANDA not defined
#endif

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpanda() {
  init_libpnmimagetypes();
  init_libdevice();
  init_libdisplay();
  init_libpgraph();
#ifdef DO_PSTATS
  init_libpstatclient();
#endif
}
