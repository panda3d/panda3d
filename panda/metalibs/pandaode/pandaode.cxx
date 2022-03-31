/**
 * @file pandaode.cxx
 * @author drose
 * @date 2000-05-16
 */

#include "pandaode.h"
#include "config_ode.h"

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpandaode() {
  init_libode();
}
