// Filename: test_pgraph.cxx
// Created by:  drose (21Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pandaNode.h"
#include "nodePath.h"

int 
main(int argc, char *argv[]) {
  NodePath h("h");
  NodePath n1("n1");
  NodePath n2("n2");

  PT(PandaNode) node = new PandaNode("node");

  NodePath t1 = h.attach_new_node(node);
  t1.reparent_to(n1);

  t1 = NodePath();

  NodePath t2 = h.attach_new_node(node);
  t2.reparent_to(n2);
  return 0;
}
