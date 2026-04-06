/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lodNodeType.h
 * @author drose
 * @date 2007-06-08
 */

#ifndef LODNODETYPE_H
#define LODNODETYPE_H

#include "pandabase.h"

BEGIN_PUBLISH

enum LODNodeType {
  LNT_pop,
  LNT_fade,
};

END_PUBLISH

EXPCL_PANDA_PGRAPHNODES std::ostream &operator << (std::ostream &out, LODNodeType lnt);
EXPCL_PANDA_PGRAPHNODES std::istream &operator >> (std::istream &in, LODNodeType &cs);

#endif
