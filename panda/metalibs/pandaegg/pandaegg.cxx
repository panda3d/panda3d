// Filename: pandaegg.cxx
// Created by:  drose (16May00)
// 
////////////////////////////////////////////////////////////////////

#include "pandaegg.h"

#include "config_egg.h"
#include "config_builder.h"
#include "config_egg2pg.h"

////////////////////////////////////////////////////////////////////
//     Function: init_libpandaegg
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpandaegg() {
  init_libegg();
  init_libbuilder();
  init_libegg2pg();
}
