/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nurbsCurve.h
 * @author drose
 * @date 1998-02-27
 */

#ifndef NURBSCURVE_H
#define NURBSCURVE_H

#include "pandabase.h"

#include "piecewiseCurve.h"
#include "nurbsCurveInterface.h"
#include "cubicCurveseg.h"
#include "epvector.h"

/**
 * A Nonuniform Rational B-Spline.
 *
 * This class is actually implemented as a PiecewiseCurve made up of several
 * CubicCurvesegs, each of which is created using the nurbs_basis() method.
 * The list of CV's and knots is kept here, within the NurbsCurve class.
 *
 * This class is the original Panda-native implementation of a NURBS curve.
 * It is typedeffed as "NurbsCurve" and performs all NURBS curve functions if
 * we do not have the NURBS++ library available.
 *
 * However, if we *do* have the NURBS++ library, another class exists, the
 * NurbsPPCurve, which is a wrapper around that library and provides some
 * additional functionality.  In that case, the other class is typedeffed to
 * "NurbsCurve" instead of this one, and performs most of the NURBS curve
 * functions.  This class then becomes vestigial.
 */
class EXPCL_PANDA_PARAMETRICS NurbsCurve : public PiecewiseCurve, public NurbsCurveInterface {
PUBLISHED:
  NurbsCurve();
  NurbsCurve(const ParametricCurve &pc);
public:
  NurbsCurve(int order, int num_cvs,
             const PN_stdfloat knots[], const LVecBase4 cvs[]);
PUBLISHED:
  virtual ~NurbsCurve();

public:
  virtual PandaNode *make_copy() const;

  // We don't need to re-publish these, since they're all published from
  // NurbsCurveInterface.
  virtual void set_order(int order);
  virtual int get_order() const;

  virtual int get_num_cvs() const;
  virtual int get_num_knots() const;

  virtual bool insert_cv(PN_stdfloat t);

  virtual bool remove_cv(int n);
  virtual void remove_all_cvs();

  virtual bool set_cv(int n, const LVecBase4 &v);
  virtual LVecBase4 get_cv(int n) const;

  virtual bool set_knot(int n, PN_stdfloat t);
  virtual PN_stdfloat get_knot(int n) const;

  virtual bool recompute();

public:
  virtual bool
  rebuild_curveseg(int rtype0, PN_stdfloat t0, const LVecBase4 &v0,
                   int rtype1, PN_stdfloat t1, const LVecBase4 &v1,
                   int rtype2, PN_stdfloat t2, const LVecBase4 &v2,
                   int rtype3, PN_stdfloat t3, const LVecBase4 &v3);

  virtual bool stitch(const ParametricCurve *a, const ParametricCurve *b);

  INLINE CubicCurveseg *get_curveseg(int ti);

  virtual NurbsCurveInterface *get_nurbs_interface();
  virtual bool convert_to_nurbs(ParametricCurve *nc) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

protected:
  virtual int append_cv_impl(const LVecBase4 &v);
  virtual bool format_egg(std::ostream &out, const std::string &name,
                          const std::string &curve_type, int indent_level) const;

  int find_cv(PN_stdfloat t);

  int _order;

  class CV {
  public:
    CV() {}
    CV(const LVecBase4 &p, PN_stdfloat t) : _p(p), _t(t) {}
    LVecBase4 _p;
    PN_stdfloat _t;
  };

  epvector<CV> _cvs;

// TypedWritable stuff
public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_NurbsCurve(const FactoryParams &params);
  virtual void write_datagram(BamWriter *manager, Datagram &me);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PiecewiseCurve::init_type();
    NurbsCurveInterface::init_type();
    register_type(_type_handle, "NurbsCurve",
                  PiecewiseCurve::get_class_type(),
                  NurbsCurveInterface::get_class_type());
 }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "nurbsCurve.I"

#endif
