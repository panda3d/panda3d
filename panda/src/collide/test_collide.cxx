// Filename: test_collide.cxx
// Created by:  drose (24Apr00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "collisionTraverser.h"
#include "collisionNode.h"
#include "collisionSphere.h"
#include "collisionPlane.h"
#include "collisionHandlerPusher.h"

#include "namedNode.h"
#include "pt_NamedNode.h"
#include "pointerTo.h"
#include "transformTransition.h"
#include "luse.h"
#include "get_rel_pos.h"
#include "renderRelation.h"

int
main(int argc, char *argv[]) {
  PT_NamedNode r = new NamedNode("r");

  PT_NamedNode a = new NamedNode("a");
  PT_NamedNode b = new NamedNode("b");

  PT(CollisionNode) aa = new CollisionNode("aa");
  PT(CollisionNode) ab = new CollisionNode("ab");
  PT(CollisionNode) ba = new CollisionNode("ba");

  RenderRelation *r_a = new RenderRelation(r, a);
  RenderRelation *r_b = new RenderRelation(r, b);

  RenderRelation *a_aa = new RenderRelation(a, aa);
  RenderRelation *a_ab = new RenderRelation(a, ab);
  RenderRelation *b_ba = new RenderRelation(b, ba);


  CollisionSphere *aa1 = new CollisionSphere(LPoint3f(0, 0, 0), 1);
  aa->add_solid(aa1);
  a_aa->set_transition(new TransformTransition(LMatrix4f::translate_mat(0, -5, 0)));

  CollisionSphere *ab1 = new CollisionSphere(LPoint3f(0, 2, 0), 1.5);
  ab->add_solid(ab1);

  Planef plane(LVector3f(0, 1, 0), LPoint3f(0, 0, 0));
  CollisionPlane *ba1 = new CollisionPlane(plane);
  ba->add_solid(ba1);

  CollisionTraverser ct;
  PT(CollisionHandlerPusher) chp = new CollisionHandlerPusher;
  chp->add_collider(aa, a_aa);
  ct.add_collider(aa, chp);

  ct.traverse(r);

  nout << "\nFrame 2:\n\n";

  ct.traverse(r);

  return (0);
}

