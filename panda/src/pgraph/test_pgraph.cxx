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
#include "textureAttrib.h"
#include "colorAttrib.h"
#include "transformState.h"
#include "texture.h"
#include "qpgeomNode.h"
#include "geomTristrip.h"
#include "geomTrifan.h"
#include "qpcullTraverser.h"
#include "cullHandler.h"

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

  qpGeomNode *g1 = new qpGeomNode("g1");
  a1->add_child(g1);

  Geom *geom1 = new GeomTristrip;
  g1->add_geom(geom1, RenderState::make_empty());
  Geom *geom2 = new GeomTrifan;
  g1->add_geom(geom2, b->get_state());

  qpGeomNode *g2 = new qpGeomNode("g2");
  b->add_child(g2);
  g2->add_geom(geom1, b->get_state());
  g2->set_transform(TransformState::make_mat(LMatrix4f::translate_mat(10, 0, 0)));

  cerr << "\n";
  list_hierarchy(root, 0);

  qpNodePath ch_g1(g1);
  cerr << ch_g1 << "\n";

  qpNodePath ch_g2(g2);
  cerr << ch_g2 << "\n";

  cerr << *ch_g1.get_transform(ch_g2) << "\n";
  cerr << *ch_g2.get_transform(ch_g1) << "\n";

  cerr << *ch_g1.get_state(ch_g2) << "\n";
  cerr << *ch_g2.get_state(ch_g1) << "\n";

  cerr << *ch_g2.get_transform(ch_g2) << "\n";
  cerr << *ch_g2.get_state(ch_g2) << "\n";

  cerr << "\n";
  return 0;
}
