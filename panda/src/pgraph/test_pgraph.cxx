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
#include "textureAttrib.h"
#include "colorAttrib.h"
#include "texture.h"

void
list_hierarchy(PandaNode *node, int indent_level) {
  node->write(cerr, indent_level);
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    list_hierarchy(cr.get_child(i), indent_level + 2);
  }
}

int 
main(int argc, char *argv[]) {
  PT(Texture) tex = new Texture;
  tex->set_name("tex");

  PT(PandaNode) root = new PandaNode("root");
  root->set_attrib(TextureAttrib::make_off());
  root->set_attrib(ColorAttrib::make_flat(Colorf(1, 0, 0, 1)));

  PandaNode *a = new PandaNode("a");
  root->add_child(a);
  a->set_attrib(TextureAttrib::make(tex));

  PandaNode *b = new PandaNode("b");
  root->add_child(b);
  b->set_attrib(ColorAttrib::make_vertex());

  PandaNode *a1 = new PandaNode("a1");
  a->add_child(a1);

  cerr << "\n";
  list_hierarchy(root, 0);

  cerr << "\nroot's attribs:\n";
  root->get_state()->write(cerr, 0);

  cerr << "\na's attribs:\n";
  a->get_state()->write(cerr, 0);

  cerr << "\nroot compose a:\n";
  CPT(RenderState) result1 = root->get_state()->compose(a->get_state());
  result1->write(cerr, 0);

  //  a->clear_state();

  cerr << "\nroot compose root:\n";
  CPT(RenderState) result2 = root->get_state()->compose(root->get_state());
  result2->write(cerr, 0);

  cerr << "\nroot compose a:\n";
  CPT(RenderState) result3 = root->get_state()->compose(a->get_state());
  result3->write(cerr, 0);

  cerr << "\na compose root:\n";
  CPT(RenderState) result4 = a->get_state()->compose(root->get_state());
  result4->write(cerr, 0);

  return 0;
}
