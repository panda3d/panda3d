/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parametricCurveCollection.h
 * @author drose
 * @date 2001-03-04
 */

#ifndef PARAMETRICCURVECOLLECTION_H
#define PARAMETRICCURVECOLLECTION_H

#include "pandabase.h"

#include "parametricCurve.h"

#include "referenceCount.h"
#include "pointerTo.h"
#include "luse.h"

#include "pvector.h"
#include "plist.h"

class ParametricCurveDrawer;

/**
 * This is a set of zero or more ParametricCurves, which may or may not be
 * related.  If they are related, the set should contain no more than one XYZ
 * curve, no more than one HPR curve, and zero or more Timewarp curves, which
 * can then be evaluated as a unit to return a single transformation matrix
 * for a given unit of time.
 */
class EXPCL_PANDA_PARAMETRICS ParametricCurveCollection : public ReferenceCount {
PUBLISHED:
  ParametricCurveCollection();
  INLINE ~ParametricCurveCollection();

  void add_curve(ParametricCurve *curve);
  void add_curve(ParametricCurve *curve, int index);
  void insert_curve(size_t index, ParametricCurve *curve);
  int add_curves(PandaNode *node);
  bool remove_curve(ParametricCurve *curve);
  void remove_curve(size_t index);
  void set_curve(size_t index, ParametricCurve *curve);
  bool has_curve(ParametricCurve *curve) const;
  void clear();
  void clear_timewarps();

  INLINE int get_num_curves() const;
  INLINE ParametricCurve *get_curve(int index) const;
  MAKE_SEQ(get_curves, get_num_curves, get_curve);

  ParametricCurve *get_xyz_curve() const;
  ParametricCurve *get_hpr_curve() const;
  ParametricCurve *get_default_curve() const;
  int get_num_timewarps() const;
  ParametricCurve *get_timewarp_curve(int n) const;
  MAKE_SEQ(get_timewarp_curves, get_num_timewarps, get_timewarp_curve);

  INLINE PN_stdfloat get_max_t() const;

  MAKE_SEQ_PROPERTY(curves, get_num_curves, get_curve, set_curve, remove_curve);
  MAKE_PROPERTY(xyz_curve, get_xyz_curve);
  MAKE_PROPERTY(hpr_curve, get_hpr_curve);
  MAKE_PROPERTY(default_curve, get_default_curve);
  MAKE_SEQ_PROPERTY(timewarp_curves, get_num_timewarps, get_timewarp_curve);
  MAKE_PROPERTY(max_t, get_max_t);

  void make_even(PN_stdfloat max_t, PN_stdfloat segments_per_unit);
  void face_forward(PN_stdfloat segments_per_unit);
  void reset_max_t(PN_stdfloat max_t);

  bool evaluate(PN_stdfloat t, LVecBase3 &xyz, LVecBase3 &hpr) const;
  bool evaluate(PN_stdfloat t, LMatrix4 &result, CoordinateSystem cs = CS_default) const;

  PN_stdfloat evaluate_t(PN_stdfloat t) const;
  INLINE bool evaluate_xyz(PN_stdfloat t, LVecBase3 &xyz) const;
  INLINE bool evaluate_hpr(PN_stdfloat t, LVecBase3 &hpr) const;

  INLINE bool adjust_xyz(PN_stdfloat t, PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  bool adjust_xyz(PN_stdfloat t, const LVecBase3 &xyz);
  INLINE bool adjust_hpr(PN_stdfloat t, PN_stdfloat h, PN_stdfloat p, PN_stdfloat r);
  bool adjust_hpr(PN_stdfloat t, const LVecBase3 &xyz);

  bool recompute();

  bool stitch(const ParametricCurveCollection *a,
              const ParametricCurveCollection *b);

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

  bool write_egg(Filename filename, CoordinateSystem cs = CS_default);
  bool write_egg(std::ostream &out, const Filename &filename, CoordinateSystem cs);

public:
  int r_add_curves(PandaNode *node);
  void register_drawer(ParametricCurveDrawer *drawer);
  void unregister_drawer(ParametricCurveDrawer *drawer);

private:
  bool determine_hpr(PN_stdfloat t, ParametricCurve *xyz_curve, LVecBase3 &hpr) const;
  void prepare_add_curve(ParametricCurve *curve);
  void prepare_remove_curve(ParametricCurve *curve);
  void redraw();

private:
  typedef pvector< PT(ParametricCurve) > ParametricCurves;
  ParametricCurves _curves;
  typedef plist<ParametricCurveDrawer *> DrawerList;
  DrawerList _drawers;
};

INLINE std::ostream &
operator << (std::ostream &out, const ParametricCurveCollection &col) {
  col.output(out);
  return out;
}

#include "parametricCurveCollection.I"

#endif
