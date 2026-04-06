/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileVertex.h
 * @author drose
 * @date 2001-06-19
 */

#ifndef XFILEVERTEX_H
#define XFILEVERTEX_H

#include "pandatoolbase.h"
#include "luse.h"

class EggVertex;
class EggPrimitive;

/**
 * This represents a single vertex associated with an XFileFace.
 */
class XFileVertex {
public:
  XFileVertex();
  void set_from_egg(EggVertex *egg_vertex, EggPrimitive *egg_poly);
  int compare_to(const XFileVertex &other) const;

  LVertexd _point;
  LTexCoordd _uv;
  LColor _color;
  bool _has_color;
  bool _has_uv;
};

#endif
