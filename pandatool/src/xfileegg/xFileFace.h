/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileFace.h
 * @author drose
 * @date 2001-06-19
 */

#ifndef XFILEFACE_H
#define XFILEFACE_H

#include "pandatoolbase.h"
#include "pvector.h"

class XFileMesh;
class EggPolygon;

/**
 * This represents a single face of an XFileMesh.
 */
class XFileFace {
public:
  XFileFace();
  void set_from_egg(XFileMesh *mesh, EggPolygon *egg_poly);

  class Vertex {
  public:
    int _vertex_index;
    int _normal_index;
  };
  typedef pvector<Vertex> Vertices;
  Vertices _vertices;

  int _material_index;
};

#endif
