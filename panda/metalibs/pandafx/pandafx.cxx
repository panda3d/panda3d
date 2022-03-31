/**
 * @file pandafx.cxx
 * @author drose
 * @date 2001-12-11
 */

#include "pandafx.h"

#include "config_distort.h"

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpandafx() {
  init_libdistort();
}
