// Filename: pandadx.cxx
// Created by:  masad (15Jan04)
// 
////////////////////////////////////////////////////////////////////

#include "pandadx9.h"

#include "config_dxgsg9.h"

////////////////////////////////////////////////////////////////////
//     Function: init_libpandadx
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpandadx9() {
  init_libdxgsg9();
}
