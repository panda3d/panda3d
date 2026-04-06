/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileNormal.h
 * @author drose
 * @date 2001-06-19
 */

#ifndef XFILENORMAL_H
#define XFILENORMAL_H

#include "pandatoolbase.h"
#include "luse.h"

class EggVertex;
class EggPrimitive;

/**
 * This represents a single normal associated with an XFileFace.  It is
 * separate from XFileVertex, because the X syntax supports a different table
 * of normals than that of vertices.
 */
class XFileNormal {
public:
  XFileNormal();
  void set_from_egg(EggVertex *egg_vertex, EggPrimitive *egg_prim);
  int compare_to(const XFileNormal &other) const;

  LNormald _normal;
  bool _has_normal;
};

#endif
