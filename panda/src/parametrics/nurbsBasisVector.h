// Filename: nurbsBasisVector.h
// Created by:  drose (03Dec02)
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

#ifndef NURBSBASISVECTOR_H
#define NURBSBASISVECTOR_H

#include "pandabase.h"
#include "luse.h"
#include "epvector.h"
#include "pmap.h"

class NurbsVertex;

////////////////////////////////////////////////////////////////////
//       Class : NurbsBasisVector
// Description : This encapsulates a series of matrices that are used
//               to represent the sequential segments of a
//               NurbsCurveEvaluator.
//
//               This is not related to NurbsCurve, CubicCurveseg or
//               any of the ParametricCurve-derived objects in this
//               module.  It is a completely parallel implementation
//               of NURBS curves, and will probably eventually replace
//               the whole ParametricCurve class hierarchy.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PARAMETRICS NurbsBasisVector {
public:
  INLINE NurbsBasisVector();
  INLINE ~NurbsBasisVector();

  INLINE int get_order() const;

  INLINE int get_num_segments() const;
  INLINE PN_stdfloat get_start_t() const;
  INLINE PN_stdfloat get_end_t() const;

  INLINE int get_vertex_index(int segment) const;
  INLINE PN_stdfloat get_from(int segment) const;
  INLINE PN_stdfloat get_to(int segment) const;
  INLINE const LMatrix4 &get_basis(int segment) const;
  INLINE PN_stdfloat scale_t(int segment, PN_stdfloat t) const;

  void clear(int order);
  void append_segment(int vertex_index, const PN_stdfloat knots[]);

  void transpose();

private:
  static LVecBase4 nurbs_blending_function(int order, int i, int j, 
                                            const PN_stdfloat knots[]);

private:
  int _order;

  class Segment {
  public:
    int _vertex_index;
    PN_stdfloat _from;
    PN_stdfloat _to;
    LMatrix4 _basis;
  };

  typedef epvector<Segment> Segments;
  Segments _segments;
};

#include "nurbsBasisVector.I"

#endif

