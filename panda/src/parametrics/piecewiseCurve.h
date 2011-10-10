// Filename: piecewiseCurve.h
// Created by:  drose (04Mar01)
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

#ifndef PIECEWISECURVE_H
#define PIECEWISECURVE_H

#include "pandabase.h"

#include "parametricCurve.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : PiecewiseCurve
// Description : A PiecewiseCurve is a curve made up of several curve
//               segments, connected in a head-to-tail fashion.  The
//               length of each curve segment in parametric space is
//               definable.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PARAMETRICS PiecewiseCurve : public ParametricCurve {
PUBLISHED:
  PiecewiseCurve();
  ~PiecewiseCurve();

public:
  // These functions are all inherited from ParametricCurve, and need
  // not be re-published.
  virtual bool is_valid() const;
  virtual PN_stdfloat get_max_t() const;

  virtual bool get_point(PN_stdfloat t, LVecBase3 &point) const;
  virtual bool get_tangent(PN_stdfloat t, LVecBase3 &tangent) const;
  virtual bool get_pt(PN_stdfloat t, LVecBase3 &point, LVecBase3 &tangent) const;
  virtual bool get_2ndtangent(PN_stdfloat t, LVecBase3 &tangent2) const;

  virtual bool adjust_point(PN_stdfloat t, PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz);
  virtual bool adjust_tangent(PN_stdfloat t, PN_stdfloat tx, PN_stdfloat ty, PN_stdfloat tz);
  virtual bool adjust_pt(PN_stdfloat t,
                         PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz,
                         PN_stdfloat tx, PN_stdfloat ty, PN_stdfloat tz);

public:
  int get_num_segs() const;

  ParametricCurve *get_curveseg(int ti);
  bool insert_curveseg(int ti, ParametricCurve *seg, PN_stdfloat tlength);

  bool remove_curveseg(int ti);
  void remove_all_curvesegs();

  PN_stdfloat get_tlength(int ti) const;
  PN_stdfloat get_tstart(int ti) const;
  PN_stdfloat get_tend(int ti) const;
  bool set_tlength(int ti, PN_stdfloat tlength);

  void make_nurbs(int order, int num_cvs,
                  const PN_stdfloat knots[], const LVecBase4 cvs[]);

  virtual bool get_bezier_segs(BezierSegs &bz_segs) const;

  virtual bool
  rebuild_curveseg(int rtype0, PN_stdfloat t0, const LVecBase4 &v0,
                   int rtype1, PN_stdfloat t1, const LVecBase4 &v1,
                   int rtype2, PN_stdfloat t2, const LVecBase4 &v2,
                   int rtype3, PN_stdfloat t3, const LVecBase4 &v3);

protected:
  bool find_curve(const ParametricCurve *&curve, PN_stdfloat &t) const;
  PN_stdfloat current_seg_range(PN_stdfloat t) const;

  class Curveseg {
  public:
    Curveseg() {}
    Curveseg(ParametricCurve *c, PN_stdfloat t) : _curve(c), _tend(t) {}

    PT(ParametricCurve) _curve;
    PN_stdfloat _tend;
  };

  pvector<Curveseg> _segs;
  int _last_ti;


// TypedWritable stuff
protected:
  virtual void write_datagram(BamWriter *manager, Datagram &me);
  void fillin(DatagramIterator &scan, BamReader *manager);
  virtual int complete_pointers(TypedWritable **plist,
                                BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParametricCurve::init_type();
    register_type(_type_handle, "PiecewiseCurve",
                  ParametricCurve::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};


#endif
