// Filename: pandadx.cxx
// Created by:  drose (15May00)
// 
////////////////////////////////////////////////////////////////////

#include "pandadx7.h"

#include "config_dxgsg7.h"

////////////////////////////////////////////////////////////////////
//     Function: init_libpandadx7
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpandadx7() {
  init_libdxgsg7();
}
