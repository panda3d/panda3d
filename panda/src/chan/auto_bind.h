// Filename: auto_bind.h
// Created by:  drose (23Feb99)
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

#ifndef AUTO_BIND_H
#define AUTO_BIND_H

#include "pandabase.h"

#include "animControl.h"
#include "animControlCollection.h"

class PandaNode;

BEGIN_PUBLISH
////////////////////////////////////////////////////////////////////
//     Function: auto_bind
//  Description: Walks the scene graph or subgraph beginning at the
//               indicated node, and attempts to bind any AnimBundles
//               found to their matching PartBundles, when possible.
//
//               The list of all resulting AnimControls created is
//               filled into controls.
////////////////////////////////////////////////////////////////////
EXPCL_PANDA_CHAN void
auto_bind(PandaNode *root_node, AnimControlCollection &controls,
          int hierarchy_match_flags = 0);
END_PUBLISH

#endif

