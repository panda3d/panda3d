// Filename: test_sequences.cxx
// Created by:  
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
#include <pt_NamedNode.h>
#include <geomNode.h>

//Relations (arcs)
#include <renderRelation.h>
#include <nodeRelation.h>

//Misc
#include <dconfig.h>
#include <framework.h>
#include <loader.h>
#include <eventHandler.h>

#include <geomprimitives.h>

#include "sequenceNode.h"

Configure(sequences);
ConfigureFn(sequences) {
}

//Framework extern variables and functions
extern PT_NamedNode render;
extern RenderRelation* first_arc;
extern PT_NamedNode lights;
extern PT_NamedNode root;
extern PT(GeomNode) geomnode;
extern PT_NamedNode cameras;
extern PT(MouseAndKeyboard) mak;

extern void set_alt_trackball(Node *trackball);

extern int framework_main(int argc, char *argv[]);
extern void (*extra_display_func)();
extern void (*define_keys)(EventHandler&);

extern PT(GraphicsWindow) main_win;

////////////////////////////////////////////////////////////////////
//     Function: keys
//       Access: Public
//  Description: Set event handlers for the various keys needed, and
//               do any initialization necessary
////////////////////////////////////////////////////////////////////
void keys(EventHandler &eh)
{
  PTA_Vertexf c1, c2, c3, c4, c5, c6;
  GeomSphere *s1, *s2, *s3, *s4, *s5, *s6;
  GeomNode *n1, *n2, *n3, *n4, *n5, *n6;

  s1 = new GeomSphere();
  s2 = new GeomSphere();
  s3 = new GeomSphere();
  s4 = new GeomSphere();
  s5 = new GeomSphere();
  s6 = new GeomSphere();

  n1 = new GeomNode("Sphere 1");
  n2 = new GeomNode("Sphere 2");
  n3 = new GeomNode("Sphere 3");
  n4 = new GeomNode("Sphere 4");
  n5 = new GeomNode("Sphere 5");
  n6 = new GeomNode("Sphere 6");

  c1.push_back(Vertexf(0,0,0));
  c1.push_back(Vertexf(0,0,4));

  c2.push_back(Vertexf(0,0,0));
  c2.push_back(Vertexf(0,0,6));

  c3.push_back(Vertexf(0,0,0));
  c3.push_back(Vertexf(0,0,8));

  c4.push_back(Vertexf(0,0,0));
  c4.push_back(Vertexf(0,0,10));

  c5.push_back(Vertexf(0,0,0));
  c5.push_back(Vertexf(0,0,8));

  c6.push_back(Vertexf(0,0,0));
  c6.push_back(Vertexf(0,0,6));

  s1->set_coords(c1, G_PER_VERTEX);
  s1->set_num_prims(1);
  s2->set_coords(c2, G_PER_VERTEX);
  s2->set_num_prims(1);
  s3->set_coords(c3, G_PER_VERTEX);
  s3->set_num_prims(1);
  s4->set_coords(c4, G_PER_VERTEX);
  s4->set_num_prims(1);
  s5->set_coords(c5, G_PER_VERTEX);
  s5->set_num_prims(1);
  s6->set_coords(c6, G_PER_VERTEX);
  s6->set_num_prims(1);

  n1->add_geom(s1);
  n2->add_geom(s2);
  n3->add_geom(s3);
  n4->add_geom(s4);
  n5->add_geom(s5);
  n6->add_geom(s6);

  PT_NamedNode sn = DCAST(NamedNode, new SequenceNode(1));

  new RenderRelation(render, sn);

  new RenderRelation(sn, n1);
  new RenderRelation(sn, n2);
  new RenderRelation(sn, n3);
  new RenderRelation(sn, n4);
  new RenderRelation(sn, n5);
  new RenderRelation(sn, n6);

  remove_arc(first_arc);
}

int main(int argc, char *argv[]) {
  define_keys = &keys;
  return framework_main(argc, argv);
}
