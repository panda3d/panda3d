// Filename: appTraverser.cxx
// Created by:  drose (25Apr00)
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

#include "appTraverser.h"
#include "config_sgraphutil.h"

#include <dftraverser.h>

////////////////////////////////////////////////////////////////////
//     Function: AppTraverser::traverse
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void AppTraverser::
traverse(Node *root) {
  if (!implicit_app_traversal) {
    df_traverse(root, *this, NullTransitionWrapper(), NullLevelState(),
                _graph_type);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AppTraverser::reached_node
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool AppTraverser::
reached_node(Node *node, NullTransitionWrapper &, NullLevelState &) {
  ArcChain bogus;
  node->app_traverse(bogus);

  return true;
}
