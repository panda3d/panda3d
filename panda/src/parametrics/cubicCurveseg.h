// Filename: cubicCurveseg.h
// Created by:  drose (04Mar01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

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


////////////////////////////////////////////////////////////////////
//       Class : CubicCurveseg
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
  virtual bool get_point(float t, LVecBase3f &point) const;
  virtual bool get_tangent(float t, LVecBase3f &tangent) const;
  virtual bool get_pt(float t, LVecBase3f &point, LVecBase3f &tangent) const;
  virtual bool get_2ndtangent(float t, LVecBase3f &tangent2) const;

public:
  CubicCurveseg();
  CubicCurveseg(const LMatrix4f &basis);
  CubicCurveseg(const BezierSeg &seg);
  CubicCurveseg(int order, const float knots[], const LVecBase4f cvs[]);

  virtual ~CubicCurveseg();

  void hermite_basis(const HermiteCurveCV &cv0,
                     const HermiteCurveCV &cv1,
                     float tlength = 1.0f);
  void bezier_basis(const BezierSeg &seg);
  void nurbs_basis(int order, const float knots[], const LVecBase4f cvs[]);

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
    float recip_h = (rational) ? 1.0f/tv.dot(Bw) : 1.0f;
    result.set(tv.dot(Bx) * recip_h,
               tv.dot(By) * recip_h,
               tv.dot(Bz) * recip_h);
  }

  void evaluate_vector(const LVecBase4f &tv, LVecBase3f &result) const {
    result.set(tv.dot(Bx),
               tv.dot(By),
               tv.dot(Bz));
  }

  virtual bool get_bezier_seg(BezierSeg &seg) const;

  static bool compute_seg(int rtype0, float t0, const LVecBase4f &v0,
                          int rtype1, float t1, const LVecBase4f &v1,
                          int rtype2, float t2, const LVecBase4f &v2,
                          int rtype3, float t3, const LVecBase4f &v3,
                          const LMatrix4f &B,
                          const LMatrix4f &Bi,
                          LMatrix4f &G);

  LVecBase4f Bx, By, Bz, Bw;
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

// This function is used internally to build the NURBS basis matrix
// based on a given knot sequence.
void compute_nurbs_basis(int order,
                         const float knots_in[],
                         LMatrix4f &basis);


#endif
