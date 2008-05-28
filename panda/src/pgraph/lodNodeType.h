// Filename: lodNodeType.h
// Created by:  drose (08Jun07)
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

#ifndef LODNODETYPE_H
#define LODNODETYPE_H

#include "pandabase.h"

BEGIN_PUBLISH

enum LODNodeType {
  LNT_pop,
  LNT_fade,
};

END_PUBLISH

EXPCL_PANDA_PGRAPH ostream &operator << (ostream &out, LODNodeType lnt);
EXPCL_PANDA_PGRAPH istream &operator >> (istream &in, LODNodeType &cs);

#endif


