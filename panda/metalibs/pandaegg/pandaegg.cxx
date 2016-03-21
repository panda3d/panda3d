/**
 * @file pandaegg.cxx
 * @author drose
 * @date 2000-05-16
 */

#include "pandaegg.h"

#include "config_egg.h"
#include "config_egg2pg.h"

// By including checkPandaVersion.h, we guarantee that runtime attempts to
// load libpandaegg.so.dll will fail if they inadvertently link with the wrong
// version of libdtool.so.dll.

#include "checkPandaVersion.h"

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpandaegg() {
  init_libegg();
  init_libegg2pg();
}
