/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file subdivSegment.h
 * @author drose
 * @date 2003-10-14
 */

#ifndef SUBDIVSEGMENT_H
#define SUBDIVSEGMENT_H

#include "pandatoolbase.h"
#include "pvector.h"
#include "vector_int.h"

/**
 * Represents a single hypothetical subdivided segment, under consideration by
 * the IsoPlacer.
 */
class SubdivSegment {
public:
  INLINE SubdivSegment(const double *cint, int f, int t);

  INLINE double get_score() const;
  INLINE double get_need() const;
  INLINE bool operator < (const SubdivSegment &other) const;

  void cut();

  const double *_cint;
  int _f, _t;
  int _num_cuts;
  vector_int _cuts;
};

#include "subdivSegment.I"

#endif
