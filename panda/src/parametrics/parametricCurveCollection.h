// Filename: parametricCurveCollection.h
// Created by:  drose (04Mar01)
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

#ifndef NODEPATHCOLLECTION_H
#define NODEPATHCOLLECTION_H

#include "pandabase.h"

#include "parametricCurve.h"

#include <referenceCount.h>
#include "pointerTo.h"
#include "luse.h"

#include "pvector.h"
#include "plist.h"

class ParametricCurveDrawer;

////////////////////////////////////////////////////////////////////
//       Class : ParametricCurveCollection
// Description : This is a set of zero or more ParametricCurves, which
//               may or may not be related.  If they are related, the
//               set should contain no more than one XYZ curve, no
//               more than one HPR curve, and zero or more Timewarp
//               curves, which can then be evaluated as a unit to
//               return a single transformation matrix for a given
//               unit of time.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ParametricCurveCollection : public ReferenceCount {
PUBLISHED:
  ParametricCurveCollection();
  INLINE ~ParametricCurveCollection();

  void add_curve(ParametricCurve *curve);
  void add_curve(ParametricCurve *curve, int index);
  int add_curves(PandaNode *node);
  bool remove_curve(ParametricCurve *curve);
  void remove_curve(int index);
  bool has_curve(ParametricCurve *curve) const;
  void clear();
  void clear_timewarps();

  INLINE int get_num_curves() const;
  INLINE ParametricCurve *get_curve(int index) const;

  ParametricCurve *get_xyz_curve() const;
  ParametricCurve *get_hpr_curve() const;
  ParametricCurve *get_default_curve() const;
  int get_num_timewarps() const;
  ParametricCurve *get_timewarp_curve(int n) const;

  INLINE float get_max_t() const;

  void make_even(float max_t, float segments_per_unit);
  void face_forward(float segments_per_unit);
  void reset_max_t(float max_t);

  bool evaluate(float t, LVecBase3f &xyz, LVecBase3f &hpr) const;
  bool evaluate(float t, LMatrix4f &result, CoordinateSystem cs = CS_default) const;

  float evaluate_t(float t) const;
  INLINE bool evaluate_xyz(float t, LVecBase3f &xyz) const;
  INLINE bool evaluate_hpr(float t, LVecBase3f &hpr) const;

  INLINE bool adjust_xyz(float t, float x, float y, float z);
  bool adjust_xyz(float t, const LVecBase3f &xyz);
  INLINE bool adjust_hpr(float t, float h, float p, float r);
  bool adjust_hpr(float t, const LVecBase3f &xyz);

  bool recompute();

  bool stitch(const ParametricCurveCollection *a,
              const ParametricCurveCollection *b);

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

  bool write_egg(Filename filename, CoordinateSystem cs = CS_default);
  bool write_egg(ostream &out, const Filename &filename, CoordinateSystem cs);

public:
  int r_add_curves(PandaNode *node);
  void register_drawer(ParametricCurveDrawer *drawer);
  void unregister_drawer(ParametricCurveDrawer *drawer);

private:
  bool determine_hpr(float t, ParametricCurve *xyz_curve, LVecBase3f &hpr) const;
  void prepare_add_curve(ParametricCurve *curve);
  void prepare_remove_curve(ParametricCurve *curve);
  void redraw();

private:
  typedef pvector< PT(ParametricCurve) > ParametricCurves;
  ParametricCurves _curves;
  typedef plist<ParametricCurveDrawer *> DrawerList;
  DrawerList _drawers;
};

INLINE ostream &
operator << (ostream &out, const ParametricCurveCollection &col) {
  col.output(out);
  return out;
}

#include "parametricCurveCollection.I"

#endif


