// Filename: auto_bind.h
// Created by:  drose (23Feb99)
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
EXPCL_PANDA void
auto_bind(PandaNode *root_node, AnimControlCollection &controls,
          int hierarchy_match_flags = 0);
END_PUBLISH

#endif

