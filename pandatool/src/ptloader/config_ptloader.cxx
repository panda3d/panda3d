// Filename: config_ptloader.cxx
// Created by:  drose (26Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "config_ptloader.h"
#include "loaderFileTypePandatool.h"

#include <fltToEggConverter.h>
#include <lwoToEggConverter.h>

#include <dconfig.h>
#include <loaderFileTypeRegistry.h>
#include <eggData.h>

ConfigureDef(config_ptloader);

ConfigureFn(config_ptloader) {
  init_libptloader();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libptloader
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libptloader() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  LoaderFileTypePandatool::init_type();

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_ptr();

  FltToEggConverter *flt = new FltToEggConverter;
  reg->register_type(new LoaderFileTypePandatool(flt));

  LwoToEggConverter *lwo = new LwoToEggConverter;
  reg->register_type(new LoaderFileTypePandatool(lwo));
}
