// Filename: curve.h
// Created by:  drose (14Mar97)
//
////////////////////////////////////////////////////////////////////
// Copyright (C) 1992,93,94,95,96,97  Walt Disney Imagineering, Inc.
//
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
////////////////////////////////////////////////////////////////////
//
#ifndef CURVE_H
#define CURVE_H

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////

#include "pandabase.h"

#include <typedef.h>
#include <list>
#include <vector>

#include "typedWriteableReferenceCount.h"
#include "namable.h"
#include "luse.h"


////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

// Parametric curve semantic types.  A parametric curve may have one
// of these types specified.  These serve as hints to the egg reader
// and writer code about the intention of this curve, and have no
// other effect on the curve.

BEGIN_PUBLISH //[
#define PCT_NONE        0
// Unspecified type.

#define PCT_XYZ         1
// A three-dimensional curve in space.

#define PCT_HPR         2
// The curve represents Euler rotation angles.

#define PCT_T           3
// A one-dimensional timewarp curve.
END_PUBLISH //]


//#define LVecBase3f LVecBase3f
//typedef LVecBase3f LVecBase3f;


// These symbols are used to define the shape of the curve segment to
// CubicCurveseg::compute_seg().

#define RT_POINT       0x01
#define RT_TANGENT     0x02
#define RT_CV          0x03
#define RT_BASE_TYPE   0xff

#define RT_KEEP_ORIG  0x100


class ParametricCurveDrawer;
class HermiteCurveCV;
class HermiteCurve;
class NurbsCurve;


////////////////////////////////////////////////////////////////////
//       Class : ParametricCurve
// Description : A virtual base class for parametric curves.
//               This encapsulates all curves in 3-d space defined
//               for a single parameter t in the range [0,get_max_t()].
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ParametricCurve : public TypedWriteableReferenceCount,
    public Namable {
PUBLISHED:
  ParametricCurve();
  virtual ~ParametricCurve();

  virtual bool is_valid() const;

  virtual double get_max_t() const;

  void set_curve_type(int type);
  int get_curve_type() const;

  void set_num_dimensions(int num);
  int get_num_dimensions() const;

  float calc_length() const;
  float calc_length(double from, double to) const;
  double compute_t(double start_t, double length_offset, double guess,
                   double threshold) const;

  bool convert_to_hermite(HermiteCurve &hc) const;
  bool convert_to_nurbs(NurbsCurve &nc) const;

  void ascii_draw() const;

  virtual bool get_point(double t, LVecBase3f &point) const=0;
  virtual bool get_tangent(double t, LVecBase3f &tangent) const=0;
  virtual bool get_pt(double t, LVecBase3f &point, LVecBase3f &tangent) const=0;
  virtual bool get_2ndtangent(double t, LVecBase3f &tangent2) const=0;

public:

  struct BezierSeg {
  public:
    LVecBase3f _v[4];
    double _t;
  };
  typedef vector<BezierSeg> BezierSegs;

  virtual void write_datagram(BamWriter *, Datagram &);

  virtual bool GetBezierSegs(BezierSegs &) const {
    return false;
  }

  virtual bool GetBezierSeg(BezierSeg &) const {
    return false;
  }

  void register_drawer(ParametricCurveDrawer *drawer);
  void unregister_drawer(ParametricCurveDrawer *drawer);

protected:
  void invalidate(double t1, double t2);
  void invalidate_all();

  float r_calc_length(double t1, double t2,
                      const LPoint3f &p1, const LPoint3f &p2,
                      float seglength) const;

  typedef list< ParametricCurveDrawer * > DrawerList;
  DrawerList _drawers;
  int _curve_type;
  int _num_dimensions;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWriteableReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "ParametricCurve",
		  TypedWriteableReferenceCount::get_class_type(),
		  Namable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};


////////////////////////////////////////////////////////////////////
//          Class : PiecewiseCurve
// Description : A PiecewiseCurve is a curve made up of several curve
//               segments, connected in a head-to-tail fashion.  The
//               length of each curve segment in parametric space is
//               definable.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PiecewiseCurve : public ParametricCurve {
PUBLISHED:
  PiecewiseCurve();
  ~PiecewiseCurve();

  virtual bool is_valid() const;
  virtual double get_max_t() const;

  virtual bool get_point(double t, LVecBase3f &point) const;
  virtual bool get_tangent(double t, LVecBase3f &tangent) const;
  virtual bool get_pt(double t, LVecBase3f &point, LVecBase3f &tangent) const;
  virtual bool get_2ndtangent(double t, LVecBase3f &tangent2) const;

  bool adjust_point(double t,
                       float px, float py, float pz);
  bool adjust_tangent(double t,
                         float tx, float ty, float tz);
  bool adjust_pt(double t,
                    float px, float py, float pz,
                    float tx, float ty, float tz);

public:
  int get_num_segs() const;

  ParametricCurve *get_curveseg(int ti);
  bool insert_curveseg(int ti, ParametricCurve *seg, double tlength);

  bool remove_curveseg(int ti);
  void remove_all_curvesegs();

  double get_tlength(int ti) const;
  double get_tstart(int ti) const;
  double get_tend(int ti) const;
  bool set_tlength(int ti, double tlength);

  void make_nurbs(int order, int num_cvs,
                  const double knots[], const LVecBase4f cvs[]);

  virtual bool GetBezierSegs(BezierSegs &bz_segs) const;

  virtual bool
  rebuild_curveseg(int rtype0, double t0, const LVecBase4f &v0,
                   int rtype1, double t1, const LVecBase4f &v1,
                   int rtype2, double t2, const LVecBase4f &v2,
                   int rtype3, double t3, const LVecBase4f &v3);

protected:
  bool find_curve(const ParametricCurve *&curve, double &t) const;
  double current_seg_range(double t) const;

  class Curveseg {
  public:
    Curveseg() {}
    Curveseg(ParametricCurve *c, double t) : _curve(c), _tend(t) {}

    int descend_pfb(void *handle);
    int store_pfb(void *handle);
    int load_pfb(void *handle);

    ParametricCurve *_curve;
    double _tend;
  };

  vector<Curveseg> _segs;
  int _last_ti;


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




////////////////////////////////////////////////////////////////////
//          Class : CubicCurveseg
// Description : A CubicCurveseg is any curve that can be completely
//               described by four 4-valued basis vectors, one for
//               each dimension in three-space, and one for the
//               homogeneous coordinate.  This includes Beziers,
//               Hermites, and NURBS.
//
//               This class encapsulates a single curve segment of the
//               cubic curve.  Normally, when we think of Bezier and
//               Hermite curves, we think of a piecewise collection of
//               such segments.
//
//               Although this class includes methods such as
//               hermite_basis() and nurbs_basis(), to generate a
//               Hermite and NURBS curve segment, respectively, only
//               the final basis vectors are stored: the product of
//               the basis matrix of the corresponding curve type, and
//               its geometry vectors.  This is the minimum
//               information needed to evaluate the curve.  However,
//               the individual CV's that were used to compute these
//               basis vectors are not retained; this might be handled
//               in a subclass (for instance, HermiteCurve).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CubicCurveseg : public ParametricCurve {
PUBLISHED:
  virtual bool get_point(double t, LVecBase3f &point) const;
  virtual bool get_tangent(double t, LVecBase3f &tangent) const;
  virtual bool get_pt(double t, LVecBase3f &point, LVecBase3f &tangent) const;
  virtual bool get_2ndtangent(double t, LVecBase3f &tangent2) const;

public:
  CubicCurveseg();
  CubicCurveseg(const LMatrix4f &basis);
  CubicCurveseg(const BezierSeg &seg);
  CubicCurveseg(int order, const double knots[], const LVecBase4f cvs[]);

  virtual ~CubicCurveseg();

  void hermite_basis(const HermiteCurveCV &cv0,
          const HermiteCurveCV &cv1,
          double tlength = 1.0);
  void bezier_basis(const BezierSeg &seg);
  void nurbs_basis(int order, const double knots[], const LVecBase4f cvs[]);

  // evaluate_point() and evaluate_vector() both evaluate the curve at
  // a given point by applying the basis vector against the vector
  // [t3 t2 t 1] (or some derivative).  The difference between the
  // two is that evaluate_point() is called only with the vector
  // [t3 t2 t 1] and computes a point in three-space and will scale by
  // the homogeneous coordinate when the curve demands it (e.g. a
  // NURBS), while evaluate_vector() is called with some derivative
  // vector like [3t2 2t 1 0] and computes a vector difference between
  // points, and will never scale by the homogeneous coordinate (which
  // would be zero anyway).

  void evaluate_point(const LVecBase4f &tv, LVecBase3f &result) const {
    double h = (rational) ? tv.dot(Bw) : 1.0;
    result.set(tv.dot(Bx) / h,
               tv.dot(By) / h,
               tv.dot(Bz) / h);
  }

  void evaluate_vector(const LVecBase4f &tv, LVecBase3f &result) const {
    result.set(tv.dot(Bx),
               tv.dot(By),
               tv.dot(Bz));
  }

  virtual bool GetBezierSeg(BezierSeg &seg) const;

  static bool compute_seg(int rtype0, double t0, const LVecBase4f &v0,
                             int rtype1, double t1, const LVecBase4f &v1,
                             int rtype2, double t2, const LVecBase4f &v2,
                             int rtype3, double t3, const LVecBase4f &v3,
                             const LMatrix4f &B,
                             const LMatrix4f &Bi,
                             LMatrix4f &G);

  LVecBase4f Bx, By, Bz, Bw;
  bool rational;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParametricCurve::init_type();
    register_type(_type_handle, "CubicCurveseg",
                  ParametricCurve::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

// This function is used internally to build the NURBS basis matrix
// based on a given knot sequence.
void compute_nurbs_basis(int order,
                         const double knots_in[],
                         LMatrix4f &basis);


#endif
