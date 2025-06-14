/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_mayaloader.cxx
 * @author drose
 * @date 2003-10-09
 */

#ifdef __MACH__
#define __OPENTRANSPORTPROVIDERS__
#endif

#include "pandatoolbase.h"
#include "loaderFileTypePandatool.h"
#include "config_mayaegg.h"
#include "mayaToEggConverter.h"

#include "dconfig.h"
#include "loaderFileTypeRegistry.h"

Configure(config_mayaloader);

void EXPCL_MISC init_libmayaloader();

ConfigureFn(config_mayaloader) {
  init_libmayaloader();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libmayaloader() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  LoaderFileTypePandatool::init_type();

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_global_ptr();

  init_libmayaegg();
  MayaToEggConverter *maya = new MayaToEggConverter;
  reg->register_type(new LoaderFileTypePandatool(maya));
}
