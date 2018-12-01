/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file hermiteCurve.h
 * @author drose
 * @date 1998-02-27
 */

#ifndef HERMITECURVE_H
#define HERMITECURVE_H

#include "piecewiseCurve.h"
#include "cubicCurveseg.h"


BEGIN_PUBLISH //[
// Hermite curve continuity types.
#define HC_CUT         1
// The curve is disconnected at this point.  All points between this and the
// following CV are not part of the curve.

#define HC_FREE        2
// Tangents are unconstrained.  The curve is continuous, but its first
// derivative is not.  This is G0 geometric continuity.

#define HC_G1          3
// Tangents are constrained to be collinear.  The curve's derivative is not
// continuous in parametric space, but its geometric slope is.  The
// distinction is mainly relevant in the context of animation along the curve
// --when crossing the join point, direction of motion will change
// continuously, but the speed of motion may change suddenly.  This is G1
// geometric continuity.

#define HC_SMOOTH     4
// Tangents are constrained to be identical.  The curve and its first
// derivative are continuous in parametric space.  When animating motion
// across the join point, speed and direction of motion will change
// continuously.  This is C1 parametric continuity.
END_PUBLISH //]

/**
 * A single CV of a Hermite curve.  Hermite curve CV's include an in and out
 * tangent, as well as a position.
 */
class EXPCL_PANDA_PARAMETRICS HermiteCurveCV {
public:
  HermiteCurveCV();
  HermiteCurveCV(const HermiteCurveCV &c);
  ~HermiteCurveCV();

  void set_point(const LVecBase3 &point) { _p = point; }
  void set_in(const LVecBase3 &in);
  void set_out(const LVecBase3 &out);
  void set_type(int type);
  void set_name(const std::string &name);

  void format_egg(std::ostream &out, int indent, int num_dimensions,
              bool show_in, bool show_out,
              PN_stdfloat scale_in, PN_stdfloat scale_out) const;

  void write_datagram(BamWriter *manager, Datagram &me) const;
  void fillin(DatagramIterator &scan, BamReader *manager);

  LVecBase3 _p, _in, _out;
  int _type;
  std::string _name;
};

/**
 * A parametric curve defined by a sequence of control vertices, each with an
 * in and out tangent.
 *
 * This class is actually implemented as a PiecewiseCurve made up of several
 * CubicCurvesegs, each of which is created using the hermite_basis() method.
 * The HermiteCurve class itself keeps its own list of the CV's that are used
 * to define the curve (since the CubicCurveseg class doesn't retain these).
 */
class EXPCL_PANDA_PARAMETRICS HermiteCurve : public PiecewiseCurve {
PUBLISHED:
  HermiteCurve();
  HermiteCurve(const ParametricCurve &pc);
  virtual ~HermiteCurve();

  int get_num_cvs() const;

  int insert_cv(PN_stdfloat t);
  int append_cv(int type, PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  inline int append_cv(int type, const LVecBase3 &v) {
    return append_cv(type, v[0], v[1], v[2]);
  }

  bool remove_cv(int n);
  void remove_all_cvs();

  bool set_cv_type(int n, int type);
  bool set_cv_point(int n, PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  inline bool set_cv_point(int n, const LVecBase3 &v) {
    return set_cv_point(n, v[0], v[1], v[2]);
  }
  bool set_cv_in(int n, PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  inline bool set_cv_in(int n, const LVecBase3 &v) {
    return set_cv_in(n, v[0], v[1], v[2]);
  }
  bool set_cv_out(int n, PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  inline bool set_cv_out(int n, const LVecBase3 &v) {
    return set_cv_out(n, v[0], v[1], v[2]);
  }
  bool set_cv_tstart(int n, PN_stdfloat tstart);
  bool set_cv_name(int n, const char *name);


  int get_cv_type(int n) const;
  const LVecBase3 &get_cv_point(int n) const;
  void get_cv_point(int n, LVecBase3 &v) const;
  const LVecBase3 &get_cv_in(int n) const;
  void get_cv_in(int n, LVecBase3 &v) const;
  const LVecBase3 &get_cv_out(int n) const;
  void get_cv_out(int n, LVecBase3 &v) const;
  PN_stdfloat get_cv_tstart(int n) const;
  std::string get_cv_name(int n) const;

  virtual void output(std::ostream &out) const;
  void write_cv(std::ostream &out, int n) const;

public:

  CubicCurveseg *get_curveseg(int ti) {
    return (CubicCurveseg *)PiecewiseCurve::get_curveseg(ti);
  }

  virtual bool
  rebuild_curveseg(int rtype0, PN_stdfloat t0, const LVecBase4 &v0,
                   int rtype1, PN_stdfloat t1, const LVecBase4 &v1,
                   int rtype2, PN_stdfloat t2, const LVecBase4 &v2,
                   int rtype3, PN_stdfloat t3, const LVecBase4 &v3);

protected:
  virtual bool format_egg(std::ostream &out, const std::string &name,
                          const std::string &curve_type, int indent_level) const;

  void invalidate_cv(int n, bool redo_all);
  int find_cv(PN_stdfloat t);
  void recompute_basis();

  pvector<HermiteCurveCV> _points;

// TypedWritable stuff
public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_HermiteCurve(const FactoryParams &params);
  virtual void write_datagram(BamWriter *manager, Datagram &me);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PiecewiseCurve::init_type();
    register_type(_type_handle, "HermiteCurve",
                  PiecewiseCurve::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
