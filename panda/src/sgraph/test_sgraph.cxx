// Filename: test_sgraph.cxx
// Created by:  mike (02Feb00)
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

#include "geomNode.h"

#include "pointerTo.h"
#include "clockObject.h"
#include "notify.h"

int main() {
  nout << "running test_sgraph" << endl;

  ClockObject *c = ClockObject::get_global_clock();
  int i;
  int count = 100000;
  double start, finish;
  PT(Node) node;

  node = new GeomNode("foo");
  start = c->get_real_time();
  for (i = 0; i < count; i++) {
    node->is_of_type(GeomNode::get_class_type());
  }
  finish = c->get_real_time();

  cerr << "test 1: " << (finish - start) / (double)count * 1000.0 
       << " ms\n";

  node = new NamedNode("foo");
  start = c->get_real_time();
  for (i = 0; i < count; i++) {
    node->is_of_type(GeomNode::get_class_type());
  }
  finish = c->get_real_time();

  cerr << "test 2: " << (finish - start) / (double)count * 1000.0
       << " ms\n";

  return 0;
}
