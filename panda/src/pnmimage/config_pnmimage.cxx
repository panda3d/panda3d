/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pnmimage.cxx
 * @author drose
 * @date 2000-03-19
 */

#include "config_pnmimage.h"
#include "pnmFileType.h"
#include "pnmFileTypeRegistry.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_PNMIMAGE)
  #error Buildsystem error: BUILDING_PANDA_PNMIMAGE not defined
#endif

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

ConfigVariableBool pfm_resize_quick
("pfm-resize-quick", true,
 PRC_DESC("Specify true to implement PfmFile::resize() with a \"quick\" filter, "
          "but only when the pfm is being downsampled (to a smaller size).  "
          "This just controls the behavior of resize(); you can "
          "always call quick_filter() explicitly."));

ConfigVariableBool pfm_resize_gaussian
("pfm-resize-gaussian", true,
 PRC_DESC("Specify true to implement PfmFile::resize() with a higher-quality "
          "Gaussian filter, or false to implement it with a faster box "
          "filter.  If pfm-resize-quick is also true, this only takes effect "
          "when the pfm is being upsampled.  This just controls the behavior "
          "of resize(); you can always call box_filter() or gaussian_filter() "
          "explicitly."));

ConfigVariableDouble pfm_resize_radius
("pfm-resize-radius", 1.0,
 PRC_DESC("Specify the default filter radius for PfmFile::resize().  "
          "This just controls the behavior of resize(); you can "
          "always call box_filter() or gaussian_filter() explicitly with "
          "a specific radius."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpnmimage() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  PNMFileType::init_type();
}
