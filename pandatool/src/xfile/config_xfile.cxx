// Filename: config_xfile.cxx
// Created by:  drose (24Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "config_xfile.h"
#include "xFile.h"
#include "xFileDataDef.h"
#include "xFileDataObject.h"
#include "xFileDataObjectArray.h"
#include "xFileDataObjectDouble.h"
#include "xFileDataObjectInteger.h"
#include "xFileDataObjectString.h"
#include "xFileDataNode.h"
#include "xFileDataNodeReference.h"
#include "xFileDataNodeTemplate.h"
#include "xFileNode.h"
#include "xFileTemplate.h"

#include "dconfig.h"

Configure(config_xfile);
NotifyCategoryDef(xfile, "");

// This is set true, typically by the user's command-line options, to
// indicate that when a X file is generated it should include all
// geometry in one big mesh, instead of preserving the hierarchy
// from the source egg file.
bool xfile_one_mesh = false;

ConfigureFn(config_xfile) {
  init_libxfile();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libxfile
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libxfile() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  XFile::init_type();
  XFileDataDef::init_type();
  XFileDataObject::init_type();
  XFileDataObjectArray::init_type();
  XFileDataObjectDouble::init_type();
  XFileDataObjectInteger::init_type();
  XFileDataObjectString::init_type();
  XFileDataNode::init_type();
  XFileDataNodeReference::init_type();
  XFileDataNodeTemplate::init_type();
  XFileNode::init_type();
  XFileTemplate::init_type();
}

