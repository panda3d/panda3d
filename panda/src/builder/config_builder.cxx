// Filename: config_builder.cxx
// Created by:  drose (29Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "config_builder.h"

#include <dconfig.h>

Configure(config_builder);
NotifyCategoryDef(builder, "");

ConfigureFn(config_builder) {
  init_libbuilder();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libbuilder
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libbuilder() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
}
