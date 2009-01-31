// Filename: pandadx.cxx
// Created by:  drose (15May00)
// 
////////////////////////////////////////////////////////////////////

#include "pandadx8.h"

#include "config_dxgsg8.h"
#include "wdxGraphicsPipe8.h"

// By including checkPandaVersion.h, we guarantee that runtime
// attempts to load libpandadx8.dll will fail if they
// inadvertently link with the wrong version of libdtool.dll.

#include "checkPandaVersion.h"

////////////////////////////////////////////////////////////////////
//     Function: init_libpandadx
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpandadx8() {
  init_libdxgsg8();
}

////////////////////////////////////////////////////////////////////
//     Function: get_pipe_type_pandadx8
//  Description: Returns the TypeHandle index of the recommended
//               graphics pipe type defined by this module.
////////////////////////////////////////////////////////////////////
int
get_pipe_type_pandadx8() {
  return wdxGraphicsPipe8::get_class_type().get_index();
}
