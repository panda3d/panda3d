/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file autoTextureScale.h
 * @author drose
 * @date 2011-11-28
 */

#ifndef AUTOTEXTURESCALE_H
#define AUTOTEXTURESCALE_H

#include "pandabase.h"

BEGIN_PUBLISH
enum AutoTextureScale {
  ATS_none,
  ATS_down,
  ATS_up,
  ATS_pad,
  ATS_unspecified,
};
END_PUBLISH

EXPCL_PANDA_PUTIL std::ostream &operator << (std::ostream &out, AutoTextureScale ats);
EXPCL_PANDA_PUTIL std::istream &operator >> (std::istream &in, AutoTextureScale &ats);

#endif
