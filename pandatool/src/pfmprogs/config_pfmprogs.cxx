/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pfmprogs.cxx
 * @author drose
 * @date 2010-12-23
 */

#include "config_pfmprogs.h"

#include "dconfig.h"

Configure(config_pfmprogs);
NotifyCategoryDef(pfm, "");

ConfigVariableDouble pfm_bba_dist
("pfm-bba-dist", "0.2 0.05",
 PRC_DESC("Specifies the point_dist and sample_radius, in UV space, for "
          "compute bba files with pfm_trans."));

ConfigureFn(config_pfmprogs) {
  init_libpfm();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpfm() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
}
