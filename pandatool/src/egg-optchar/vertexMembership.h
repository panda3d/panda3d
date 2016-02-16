/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexMembership.h
 * @author drose
 * @date 2003-07-21
 */

#ifndef VERTEXMEMBERSHIP_H
#define VERTEXMEMBERSHIP_H

#include "pandatoolbase.h"

#include "pvector.h"

class EggGroup;

/**
 * This class is used to help EggOptchar quantize the membership of one vertex
 * among its various groups.
 */
class VertexMembership {
public:
  INLINE VertexMembership(EggGroup *group, double membership);
  INLINE VertexMembership(const VertexMembership &copy);
  INLINE void operator = (const VertexMembership &copy);

  INLINE bool operator < (const VertexMembership &other) const;

  EggGroup *_group;
  double _membership;
};

typedef pvector<VertexMembership> VertexMemberships;

#include "vertexMembership.I"

#endif
