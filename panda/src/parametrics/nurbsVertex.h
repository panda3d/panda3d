// Filename: nurbsVertex.h
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

#ifndef NURBSVERTEX_H
#define NURBSVERTEX_H

#include "pandabase.h"
#include "luse.h"
#include "nodePath.h"
#include "pmap.h"

////////////////////////////////////////////////////////////////////
//       Class : NurbsVertex
// Description : This represents a single control vertex in a
//               NurbsEvaluator.  It may be relative to a particular
//               coordinate space; or its coordinate space may be
//               unspecified.
//
//               This is not related to NurbsCurve, ClassicNurbsCurve,
//               CubicCurveseg or any of the ParametricCurve-derived
//               objects in this module.  It is a completely parallel
//               implementation of NURBS curves, and will probably
//               eventually replace the whole ParametricCurve class
//               hierarchy.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NurbsVertex {
public:
  INLINE NurbsVertex();
  INLINE NurbsVertex(const NurbsVertex &copy);
  INLINE void operator = (const NurbsVertex &copy);
  INLINE ~NurbsVertex();

  INLINE void set_vertex(const LVecBase4f &vertex);
  INLINE const LVecBase4f &get_vertex() const;

  INLINE void set_space(const NodePath &space);
  INLINE void set_space(const string &space);
  INLINE NodePath get_space(const NodePath &rel_to) const;

  void set_extended_vertex(int d, float value);
  float get_extended_vertex(int d) const;

private:
  LVecBase4f _vertex;
  NodePath _space;
  string _space_path;
  typedef pmap<int, float> Extended;
  Extended _extended;
};

#include "nurbsVertex.I"

#endif

