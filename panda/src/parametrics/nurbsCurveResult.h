// Filename: nurbsCurveResult.h
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

#ifndef NURBSCURVERESULT_H
#define NURBSCURVERESULT_H

#include "pandabase.h"
#include "referenceCount.h"
#include "nurbsMatrixVector.h"

////////////////////////////////////////////////////////////////////
//       Class : NurbsCurveResult
// Description : The result of a NurbsCurveEvaluator.  This object
//               represents a curve in a particular coordinate space.
//               It can return the point and/or tangent to the curve
//               at any point.
//
//               This is not related to NurbsCurve, ClassicNurbsCurve,
//               CubicCurveseg or any of the ParametricCurve-derived
//               objects in this module.  It is a completely parallel
//               implementation of NURBS curves, and will probably
//               eventually replace the whole ParametricCurve class
//               hierarchy.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NurbsCurveResult : public ReferenceCount {
public:
  NurbsCurveResult(const NurbsMatrixVector &basis, int order,
                   const LVecBase4f verts[], int num_vertices);

PUBLISHED:
  INLINE ~NurbsCurveResult();

  INLINE float get_start_t() const;
  INLINE float get_end_t() const;

  INLINE bool eval_point(float t, LPoint3f &point);
  
  INLINE int get_num_segments() const;
  void eval_segment_point(int segment, float t, LPoint3f &point) const;
  
private:
  int find_segment(float t);
  int r_find_segment(float t, int top, int bot) const;

  NurbsMatrixVector _prod;

  int _last_segment;
  float _last_from;
  float _last_to;
};

#include "nurbsCurveResult.I"

#endif

