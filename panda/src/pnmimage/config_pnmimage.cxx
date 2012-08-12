// Filename: config_pnmimage.cxx
// Created by:  drose (19Mar00)
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

#include "config_pnmimage.h"
#include "pnmFileType.h"
#include "pnmFileTypeRegistry.h"

#include "dconfig.h"

Configure(config_pnmimage);
NotifyCategoryDef(pnmimage, "");

ConfigureFn(config_pnmimage) {
  init_libpnmimage();
}

ConfigVariableBool pfm_force_littleendian
("pfm-force-littleendian", false,
 PRC_DESC("This forces a pfm file to be read as a sequence of little-endian "
          "floats, even if its scale factor is given as a positive number."));

ConfigVariableBool pfm_reverse_dimensions
("pfm-reverse-dimensions", false,
 PRC_DESC("Understands that the width and height of a pfm file are given "
          "backwards, in the form height width instead of width height, "
          "on input.  Does not affect output, which is always written width height."));

ConfigVariableInt pfm_vis_max_vertices
("pfm-vis-max-vertices", 65535,
 PRC_DESC("Specifies the maximum number of vertex entries that may appear in "
          "a single generated mesh.  If the mesh would require more than that, "
          "the mesh is subdivided into smaller pieces."));

ConfigVariableInt pfm_vis_max_indices
("pfm-vis-max-indices", 1048576,
 PRC_DESC("Specifies the maximum number of vertex references that may appear in "
          "a single generated mesh.  If the mesh would require more than that, "
          "the mesh is subdivided into smaller pieces."));

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
