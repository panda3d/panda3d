/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariable.cxx
 * @author drose
 * @date 2004-10-18
 */

#include "configVariable.h"
#include "config_prc.h"

/**
 * Displays a suitable error message when an unconstructed ConfigVariable is
 * attempted to be used.  This normally indicates a static-init ordering
 * issue.
 */
void ConfigVariable::
report_unconstructed() const {
  prc_cat->error()
    << "ConfigVariable " << this
    << " accessed before its constructor has run!\n";
  record_unconstructed();
}
