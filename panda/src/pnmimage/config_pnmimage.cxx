// Filename: config_pnmimage.cxx
// Created by:  drose (19Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "config_pnmimage.h"
#include "pnmFileType.h"

#include <dconfig.h>

Configure(config_pnmimage);
NotifyCategoryDef(pnmimage, "");

ConfigureFn(config_pnmimage) {
  init_libpnmimage();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libpnmimage
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpnmimage() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  PNMFileType::init_type();
}
