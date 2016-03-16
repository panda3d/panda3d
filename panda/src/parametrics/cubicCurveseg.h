/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cubicCurveseg.h
 * @author drose
 * @date 2001-03-04
 */

#ifndef CUBICCURVESEG_H
#define CUBICCURVESEG_H

#include "pandabase.h"

#include "parametricCurve.h"


// These symbols are used to define the shape of the curve segment to
// CubicCurveseg::compute_seg().

#define RT_POINT       0x01
#define RT_TANGENT     0x02
#define RT_CV          0x03
#define RT_BASE_TYPE   0xff

#define RT_KEEP_ORIG  0x100


/**
 * A CubicCurveseg is any curve that can be completely described by four
 * 4-valued basis vectors, one for each dimension in three-space, and one for
 * the homogeneous coordinate.  This includes Beziers, Hermites, and NURBS.
 *
 * This class encapsulates a single curve segment of the cubic curve.
 * Normally, when we think of Bezier and Hermite curves, we think of a
 * piecewise collection of such segments.
 *
 * Although this class includes methods such as hermite_basis() and
 * nurbs_basis(), to generate a Hermite and NURBS curve segment, respectively,
 * only the final basis vectors are stored: the product of the basis matrix of
 * the corresponding curve type, and its geometry vectors.  This is the
 * minimum information needed to evaluate the curve.  However, the individual
 * CV's that were used to compute these basis vectors are not retained; this
 * might be handled in a subclass (for instance, HermiteCurve).
 */
class EXPCL_PANDA_PARAMETRICS CubicCurveseg : public ParametricCurve {
PUBLISHED:
  virtual bool get_point(PN_stdfloat t, LVecBase3 &point) const;
  virtual bool get_tangent(PN_stdfloat t, LVecBase3 &tangent) const;
  virtual bool get_pt(PN_stdfloat t, LVecBase3 &point, LVecBase3 &tangent) const;
  virtual bool get_2ndtangent(PN_stdfloat t, LVecBase3 &tangent2) const;

public:
  CubicCurveseg();
  CubicCurveseg(const LMatrix4 &basis);
  CubicCurveseg(const BezierSeg &seg);
  CubicCurveseg(int order, const PN_stdfloat knots[], const LVecBase4 cvs[]);

  virtual ~CubicCurveseg();

  void hermite_basis(const HermiteCurveCV &cv0,
                     const HermiteCurveCV &cv1,
                     PN_stdfloat tlength = 1.0f);
  void bezier_basis(const BezierSeg &seg);
  void nurbs_basis(int order, const PN_stdfloat knots[], const LVecBase4 cvs[]);

/*
 * evaluate_point() and evaluate_vector() both evaluate the curve at a given
 * point by applying the basis vector against the vector [t3 t2 t 1] (or some
 * derivative).  The difference between the two is that evaluate_point() is
 * called only with the vector [t3 t2 t 1] and computes a point in three-space
 * and will scale by the homogeneous coordinate when the curve demands it
 * (e.g.  a NURBS), while evaluate_vector() is called with some derivative
 * vector like [3t2 2t 1 0] and computes a vector difference between points,
 * and will never scale by the homogeneous coordinate (which would be zero
 * anyway).
 */

  void evaluate_point(const LVecBase4 &tv, LVecBase3 &result) const {
    PN_stdfloat recip_h = (rational) ? 1.0f/tv.dot(Bw) : 1.0f;
    result.set(tv.dot(Bx) * recip_h,
               tv.dot(By) * recip_h,
               tv.dot(Bz) * recip_h);
  }

  void evaluate_vector(const LVecBase4 &tv, LVecBase3 &result) const {
    result.set(tv.dot(Bx),
               tv.dot(By),
               tv.dot(Bz));
  }

  virtual bool get_bezier_seg(BezierSeg &seg) const;

  static bool compute_seg(int rtype0, PN_stdfloat t0, const LVecBase4 &v0,
                          int rtype1, PN_stdfloat t1, const LVecBase4 &v1,
                          int rtype2, PN_stdfloat t2, const LVecBase4 &v2,
                          int rtype3, PN_stdfloat t3, const LVecBase4 &v3,
                          const LMatrix4 &B,
                          const LMatrix4 &Bi,
                          LMatrix4 &G);

  LVecBase4 Bx, By, Bz, Bw;
  bool rational;


// TypedWritable stuff
public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_CubicCurveseg(const FactoryParams &params);
  virtual void write_datagram(BamWriter *manager, Datagram &me);
  void fillin(DatagramIterator &scan, BamReader *manager);

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

// This function is used internally to build the NURBS basis matrix based on a
// given knot sequence.
void compute_nurbs_basis(int order,
                         const PN_stdfloat knots_in[],
                         LMatrix4 &basis);


#endif
