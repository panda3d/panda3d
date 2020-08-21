/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMakeTube.h
 * @author drose
 * @date 2003-10-01
 */

#ifndef EGGMAKETUBE_H
#define EGGMAKETUBE_H

#include "pandatoolbase.h"

#include "eggMakeSomething.h"

class EggGroup;
class EggVertexPool;
class EggVertex;

/**
 * A program to generate an egg file representing a tube model, similar in
 * shape to a CollisionCapsule.
 */
class EggMakeTube : public EggMakeSomething {
public:
  EggMakeTube();

  void run();

private:
  EggVertex *calc_sphere1_vertex(int ri, int si);
  EggVertex *calc_sphere2_vertex(int ri, int si);
  EggVertex *calc_tube_vertex(int ri, int si);
  void add_polygon(EggVertex *a, EggVertex *b, EggVertex *c, EggVertex *d);

private:
  double _point_a[3];
  double _point_b[3];
  bool _got_point_b;
  double _radius;
  int _num_slices;
  int _num_crings;
  int _num_trings;

  double _length;
  EggGroup *_group;
  EggVertexPool *_vpool;
};

#endif
