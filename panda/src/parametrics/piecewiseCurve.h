// Filename: piecewiseCurve.h
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
class EXPCL_PANDA PiecewiseCurve : public ParametricCurve {
PUBLISHED:
  PiecewiseCurve();
  ~PiecewiseCurve();

public:
  // These functions are all inherited from ParametricCurve, and need
  // not be re-published.
  virtual bool is_valid() const;
  virtual float get_max_t() const;

  virtual bool get_point(float t, LVecBase3f &point) const;
  virtual bool get_tangent(float t, LVecBase3f &tangent) const;
  virtual bool get_pt(float t, LVecBase3f &point, LVecBase3f &tangent) const;
  virtual bool get_2ndtangent(float t, LVecBase3f &tangent2) const;

  virtual bool adjust_point(float t, float px, float py, float pz);
  virtual bool adjust_tangent(float t, float tx, float ty, float tz);
  virtual bool adjust_pt(float t,
                         float px, float py, float pz,
                         float tx, float ty, float tz);

public:
  int get_num_segs() const;

  ParametricCurve *get_curveseg(int ti);
  bool insert_curveseg(int ti, ParametricCurve *seg, float tlength);

  bool remove_curveseg(int ti);
  void remove_all_curvesegs();

  float get_tlength(int ti) const;
  float get_tstart(int ti) const;
  float get_tend(int ti) const;
  bool set_tlength(int ti, float tlength);

  void make_nurbs(int order, int num_cvs,
                  const float knots[], const LVecBase4f cvs[]);

  virtual bool get_bezier_segs(BezierSegs &bz_segs) const;

  virtual bool
  rebuild_curveseg(int rtype0, float t0, const LVecBase4f &v0,
                   int rtype1, float t1, const LVecBase4f &v1,
                   int rtype2, float t2, const LVecBase4f &v2,
                   int rtype3, float t3, const LVecBase4f &v3);

protected:
  bool find_curve(const ParametricCurve *&curve, float &t) const;
  float current_seg_range(float t) const;

  class Curveseg {
  public:
    Curveseg() {}
    Curveseg(ParametricCurve *c, float t) : _curve(c), _tend(t) {}

    PT(ParametricCurve) _curve;
    float _tend;
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
