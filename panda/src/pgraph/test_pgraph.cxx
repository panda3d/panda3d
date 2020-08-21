/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_pgraph.cxx
 * @author drose
 * @date 2002-02-21
 */

#include "pandaNode.h"
#include "nodePath.h"
#include "nodePathCollection.h"
#include "findApproxLevelEntry.h"
#include "clockObject.h"

using std::cerr;

NodePath
build_tree(const std::string &name, int depth) {
  NodePath node(name);
  if (depth > 1) {
    for (int i = 0; i < 3; i++) {
      char letter = 'a' + i;
      std::string child_name = name + std::string(1, letter);
      NodePath child = build_tree(child_name, depth - 1);
      child.reparent_to(node);
    }
  }

  return node;
}

int
main(int argc, char *argv[]) {

  // Build up a tree of height 6.  Each level has three children, so that
  // there are 3^5 + 3^4 + 3^3 + 3^2 + 3^1 + 3^0 = 364 total nodes.

  NodePath root = build_tree("a", 6);

  NodePath abba = root.find("**/abba");
  cerr << abba << "\n";
  cerr << "entries allocated: " << FindApproxLevelEntry::get_num_ever_allocated() << "\n";

  NodePath abaab = root.find("ab/aba/abaa/abaab");
  cerr << abaab << "\n";
  cerr << "entries allocated: " << FindApproxLevelEntry::get_num_ever_allocated() << "\n";

  NodePathCollection col = root.find_all_matches("**");
  cerr << col << "\n";
  cerr << "entries allocated: " << FindApproxLevelEntry::get_num_ever_allocated() << "\n";

  // Now time a bunch of find operations.
  ClockObject *clock = ClockObject::get_global_clock();
  static const int num_ops = 10000;

  volatile double start, end;
  {
    start = clock->get_real_time();
    for (int i = 0; i < num_ops; i++) {
      root.find_all_matches("**/aabca");
    }
    end = clock->get_real_time();
  }

  double avg_time = (end - start) / (double)num_ops;
  cerr << "Find operation took " << avg_time * 1000 << " ms\n";
  cerr << "entries allocated: " << FindApproxLevelEntry::get_num_ever_allocated() << "\n";

  return 0;
}
