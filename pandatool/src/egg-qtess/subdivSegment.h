// Filename: subdivSegment.h
// Created by:  drose (14Oct03)
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

#ifndef SUBDIVSEGMENT_H
#define SUBDIVSEGMENT_H

#include "pandatoolbase.h"
#include "pvector.h"
#include "vector_int.h"

////////////////////////////////////////////////////////////////////
//       Class : SubdivSegment
// Description : Represents a single hypothetical subdivided segment,
//               under consideration by the IsoPlacer.
////////////////////////////////////////////////////////////////////
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

