// Filename: nurbsMatrixVector.h
// Created by:  drose (03Dec02)
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

#ifndef NURBSMATRIXVECTOR_H
#define NURBSMATRIXVECTOR_H

#include "pandabase.h"
#include "luse.h"
#include "pvector.h"
#include "pmap.h"

class NurbsVertex;

////////////////////////////////////////////////////////////////////
//       Class : NurbsMatrixVector
// Description : This encapsulates a series of matrices that are used
//               to represent the sequential segments of a
//               NurbsCurveEvaluator.
//
//               This is not related to NurbsCurve, ClassicNurbsCurve,
//               CubicCurveseg or any of the ParametricCurve-derived
//               objects in this module.  It is a completely parallel
//               implementation of NURBS curves, and will probably
//               eventually replace the whole ParametricCurve class
//               hierarchy.
////////////////////////////////////////////////////////////////////
class NurbsMatrixVector {
public:
  INLINE NurbsMatrixVector();
  INLINE ~NurbsMatrixVector();

  INLINE int get_order() const;

  INLINE int get_num_segments() const;
  INLINE float get_start_t() const;
  INLINE float get_end_t() const;

  INLINE int get_vertex_index(int segment) const;
  INLINE float get_from(int segment) const;
  INLINE float get_to(int segment) const;
  INLINE const LMatrix4f &get_basis(int segment) const;
  INLINE float scale_t(int segment, float t) const;

  void clear(int order);
  void append_segment(int vertex_index, const float knots[]);

private:
  static LVecBase4f nurbs_blending_function(int order, int i, int j, 
                                            const float knots[]);

private:
  int _order;

  class Segment {
  public:
    int _vertex_index;
    float _from;
    float _to;
    LMatrix4f _basis;
  };

  typedef pvector<Segment> Segments;
  Segments _segments;
};

#include "nurbsMatrixVector.I"

#endif

