// Filename: config_ptloader.cxx
// Created by:  drose (26Apr01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "config_ptloader.h"
#include "loaderFileTypePandatool.h"

#include "config_lwo.h"
#include "fltToEggConverter.h"
#include "config_flt.h"
#include "lwoToEggConverter.h"

/*
#ifdef HAVE_DX
#include "config_xfile.h"
#include "xFileToEggConverter.h"
#endif
*/

/*
#ifdef HAVE_MAYA
#include "config_mayaegg.h"
#include "mayaToEggConverter.h"
#endif
*/

#include "dconfig.h"
#include "loaderFileTypeRegistry.h"
#include "eggData.h"

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

  init_liblwo();
  FltToEggConverter *flt = new FltToEggConverter;
  reg->register_type(new LoaderFileTypePandatool(flt));

  init_libflt();
  LwoToEggConverter *lwo = new LwoToEggConverter;
  reg->register_type(new LoaderFileTypePandatool(lwo));

  /*
#ifdef HAVE_DX
  init_libxfile();
  XFileToEggConverter *xfile = new XFileToEggConverter;
  reg->register_type(new LoaderFileTypePandatool(xfile));
#endif
  */

  /*
#ifdef HAVE_MAYA
  init_libmayaegg();
  MayaToEggConverter *maya = new MayaToEggConverter;
  reg->register_type(new LoaderFileTypePandatool(maya));
#endif
  */
}
