// Filename: auto_bind.h
// Created by:  drose (23Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef AUTO_BIND_H
#define AUTO_BIND_H

#include <pandabase.h>

#include "animControl.h"
#include "animControlCollection.h"

class Node;

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
auto_bind(Node *root_node, AnimControlCollection &controls,
	  int hierarchy_match_flags = 0);
END_PUBLISH

#endif

