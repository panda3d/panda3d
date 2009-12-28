// Filename: pandaphysx.cxx
// Created by:  pratt (20Apr2006)
// 
////////////////////////////////////////////////////////////////////

#include "pandaphysx.h"
#include "config_physx.h"

// By including checkPandaVersion.h, we guarantee that runtime
// attempts to load libpandaphysx.so/.dll will fail if they
// inadvertently link with the wrong version of libdtool.so/.dll.

#include "checkPandaVersion.h"

////////////////////////////////////////////////////////////////////
//     Function: init_libpandaphysx
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpandaphysx() {
  init_libphysx();
}
