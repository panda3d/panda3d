// Filename: test_pgraph.cxx
// Created by:  drose (21Feb02)
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

#include "pandaNode.h"
#include "qpnodePath.h"

int 
main(int argc, char *argv[]) {
  qpNodePath h("h");
  qpNodePath n1("n1");
  qpNodePath n2("n2");

  PT(PandaNode) node = new PandaNode("node");

  qpNodePath t1 = h.attach_new_node(node);
  t1.reparent_to(n1);

  t1 = qpNodePath();

  qpNodePath t2 = h.attach_new_node(node);
  t2.reparent_to(n2);
  return 0;
}
