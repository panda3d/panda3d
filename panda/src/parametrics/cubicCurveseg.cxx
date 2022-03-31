/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cubicCurveseg.cxx
 * @author drose
 * @date 2001-03-04
 */

#include "piecewiseCurve.h"

#include "config_parametrics.h"
#include "hermiteCurve.h"

#include "datagram.h"
#include "datagramIterator.h"
#include "bamWriter.h"
#include "bamReader.h"

TypeHandle CubicCurveseg::_type_handle;

/**
 *
 */
CubicCurveseg::
CubicCurveseg() {
}

/**
 * Creates the curveseg given the four basis vectors (the columns of the
 * matrix) explicitly.
 */
CubicCurveseg::
CubicCurveseg(const LMatrix4 &basis) {
  Bx = basis.get_col(0);
  By = basis.get_col(1);
  Bz = basis.get_col(2);
  Bw = basis.get_col(3);
  rational = true;
}


/**
 * Creates the curveseg as a Bezier segment.
 */
CubicCurveseg::
CubicCurveseg(const BezierSeg &seg) {
  bezier_basis(seg);
}


/**
 * Creates the curveseg as a NURBS segment.  See nurbs_basis for a description
 * of the parameters.
 */
CubicCurveseg::
CubicCurveseg(int order, const PN_stdfloat knots[], const LVecBase4 cvs[]) {
  nurbs_basis(order, knots, cvs);
}

/**
 *
 */
CubicCurveseg::
~CubicCurveseg() {
}



/**
 * Computes the surface point at a given parametric point t.
 */
bool CubicCurveseg::
get_point(PN_stdfloat t, LVecBase3 &point) const {
  PN_stdfloat t_sqrd = t*t;
  evaluate_point(LVecBase4(t*t_sqrd, t_sqrd, t, 1.0f), point);
  return true;
}

/**
 * Computes the surface tangent at a given parametric point t.
 */
bool CubicCurveseg::
get_tangent(PN_stdfloat t, LVecBase3 &tangent) const {
  evaluate_vector(LVecBase4(3.0f*t*t, 2.0f*t, 1.0f, 0.0f), tangent);
  return true;
}

/**
 * Simultaneously computes the point and the tangent at the given parametric
 * point.
 */
bool CubicCurveseg::
get_pt(PN_stdfloat t, LVecBase3 &point, LVecBase3 &tangent) const {
  PN_stdfloat t_sqrd=t*t;
  evaluate_point(LVecBase4(t*t_sqrd, t_sqrd, t, 1.0f), point);
  evaluate_vector(LVecBase4(3.0f*t_sqrd, /*2.0f*t*/t+t, 1.0f, 0.0f), tangent);
  return true;
}

/**
 * Computes the surface 2nd-order tangent at a given parametric point t.
 */
bool CubicCurveseg::
get_2ndtangent(PN_stdfloat t, LVecBase3 &tangent2) const {
  evaluate_vector(LVecBase4(6.0f*t, 2.0f, 0.0f, 0.0f), tangent2);
  return true;
}


/**
 * Defines the curve segment as a Hermite.  This only sets up the basis
 * vectors, so the curve will be computed correctly; it does not retain the
 * CV's.
 */
void CubicCurveseg::
hermite_basis(const HermiteCurveCV &cv0,
              const HermiteCurveCV &cv1,
              PN_stdfloat tlength) {
  static LMatrix4
    Mh( 2.0f, -3.0f, 0.0f, 1.0f,
       -2.0f,  3.0f, 0.0f, 0.0f,
        1.0f, -2.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 0.0f);

  LVecBase4 Gx(cv0._p[0], cv1._p[0],
                cv0._out[0]*tlength, cv1._in[0]*tlength);
  LVecBase4 Gy(cv0._p[1], cv1._p[1],
                cv0._out[1]*tlength, cv1._in[1]*tlength);
  LVecBase4 Gz(cv0._p[2], cv1._p[2],
                cv0._out[2]*tlength, cv1._in[2]*tlength);

  Bx = Gx * Mh;
  By = Gy * Mh;
  Bz = Gz * Mh;
  rational = false;
}

/**
 * Defines the curve segment as a Bezier.  This only sets up the basis
 * vectors, so the curve will be computed correctly; it does not retain the
 * CV's.
 */
void CubicCurveseg::
bezier_basis(const BezierSeg &seg) {
  static LMatrix4
    Mb(-1.0f,  3.0f, -3.0f, 1.0f,
        3.0f, -6.0f,  3.0f, 0.0f,
       -3.0f,  3.0f,  0.0f, 0.0f,
        1.0f,  0.0f,  0.0f, 0.0f);

  LVecBase4 Gx(seg._v[0][0], seg._v[1][0], seg._v[2][0], seg._v[3][0]);
  LVecBase4 Gy(seg._v[0][1], seg._v[1][1], seg._v[2][1], seg._v[3][1]);
  LVecBase4 Gz(seg._v[0][2], seg._v[1][2], seg._v[2][2], seg._v[3][2]);

  Bx = Gx * Mb;
  By = Gy * Mb;
  Bz = Gz * Mb;
  rational = false;
}

static LVecBase4
nurbs_blending_function(int order, int i, int j,
                        const PN_stdfloat knots[]) {
  // This is doubly recursive.  Ick.
  LVecBase4 r;

  if (j==1) {
    if (i==order-1 && knots[i] < knots[i+1]) {
      r.set(0.0f, 0.0f, 0.0f, 1.0f);
    } else {
      r.set(0.0f, 0.0f, 0.0f, 0.0f);
    }

  } else {
    LVecBase4 bi0 = nurbs_blending_function(order, i, j-1, knots);
    LVecBase4 bi1 = nurbs_blending_function(order, i+1, j-1, knots);

    PN_stdfloat d0 = knots[i+j-1] - knots[i];
    PN_stdfloat d1 = knots[i+j] - knots[i+1];

    // First term.  Division by zero is defined to equal zero.
    if (d0 != 0.0f) {
      if (d1 != 0.0f) {
        r = bi0 / d0 - bi1 / d1;
      } else {
        r = bi0 / d0;
      }

    } else if (d1 != 0.0f) {
      r = - bi1 / d1;

    } else {
      r.set(0.0f, 0.0f, 0.0f, 0.0f);
    }

    // scale by t.
    r[0] = r[1];
    r[1] = r[2];
    r[2] = r[3];
    r[3] = 0.0f;

    // Second term.
    if (d0 != 0.0f) {
      if (d1 != 0.0f) {
        r += bi0 * (- knots[i] / d0) + bi1 * (knots[i+j] / d1);
      } else {
        r += bi0 * (- knots[i] / d0);
      }

    } else if (d1 != 0.0f) {
      r += bi1 * (knots[i+j] / d1);
    }
  }

  return r;
}

void
compute_nurbs_basis(int order,
                    const PN_stdfloat knots_in[],
                    LMatrix4 &basis) {
  int i;

  // Scale the supplied knots to the range 0..1.
  PN_stdfloat knots[8];
  PN_stdfloat mink = knots_in[order-1];
  PN_stdfloat maxk = knots_in[order];

  if (mink==maxk) {
    // Huh.  What were you thinking?  This is a trivial NURBS.
    parametrics_cat->warning()
      << "Trivial NURBS curve specified." << std::endl;
    memset((void *)&basis, 0, sizeof(LMatrix4));
    return;
  }

  for (i = 0; i<2*order; i++) {
    knots[i] = (knots_in[i] - mink) / (maxk-mink);
  }


  LVecBase4 b[4];
  for (i = 0; i<order; i++) {
    b[i] = nurbs_blending_function(order, i, order, knots);
  }

  for (i = 0; i<order; i++) {
    basis.set_row(i, b[i]);
  }

  for (i=order; i<4; i++) {
    basis.set_row(i, LVecBase4::zero());
  }
}



/**
 * Defines the curve segment as a NURBS.  Order is one more than the degree,
 * and must be 1, 2, 3, or 4; knots is an array of order*2 values, and cvs is
 * an array of order values.
 */
void CubicCurveseg::
nurbs_basis(int order, const PN_stdfloat knots[], const LVecBase4 cvs[]) {
  assert(order>=1 && order<=4);

  LMatrix4 B;
  compute_nurbs_basis(order, knots, B);

  // Create a local copy of our CV's, so we can zero out the unused elements.
  LVecBase4 c[4];
  for (int i = 0; i < 4; i++) {
    c[i] = (i<order) ? cvs[i] : LVecBase4(0.0f, 0.0f, 0.0f, 0.0f);
  }

  Bx = LVecBase4(c[0][0], c[1][0], c[2][0], c[3][0]) * B;
  By = LVecBase4(c[0][1], c[1][1], c[2][1], c[3][1]) * B;
  Bz = LVecBase4(c[0][2], c[1][2], c[2][2], c[3][2]) * B;
  Bw = LVecBase4(c[0][3], c[1][3], c[2][3], c[3][3]) * B;

  rational = true;
}

/**
 * Fills the BezierSeg structure with a description of the curve segment as a
 * Bezier, if possible, but does not change the _t member of the structure.
 * Returns true if successful, false otherwise.
 */
bool CubicCurveseg::
get_bezier_seg(BezierSeg &seg) const {
  static LMatrix4
    Mbi(0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f/3.0f, 1.0f,
        0.0f, 1.0f/3.0f, 2.0f/3.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f);

  LVecBase4 Gx = Bx * Mbi;
  LVecBase4 Gy = By * Mbi;
  LVecBase4 Gz = Bz * Mbi;

  if (rational) {
    LVecBase4 Gw = Bw * Mbi;
    seg._v[0].set(Gx[0]/Gw[0], Gy[0]/Gw[0], Gz[0]/Gw[0]);
    seg._v[1].set(Gx[1]/Gw[1], Gy[1]/Gw[1], Gz[1]/Gw[1]);
    seg._v[2].set(Gx[2]/Gw[2], Gy[2]/Gw[2], Gz[2]/Gw[2]);
    seg._v[3].set(Gx[3]/Gw[3], Gy[3]/Gw[3], Gz[3]/Gw[3]);
  } else {
    seg._v[0].set(Gx[0], Gy[0], Gz[0]);
    seg._v[1].set(Gx[1], Gy[1], Gz[1]);
    seg._v[2].set(Gx[2], Gy[2], Gz[2]);
    seg._v[3].set(Gx[3], Gy[3], Gz[3]);
  }

  return true;
}

// We need this operator since Performer didn't supply it.
inline LVecBase4
col_mult(const LMatrix4 &M, const LVecBase4 &v) {
  return LVecBase4(M(0,0)*v[0] + M(0,1)*v[1] + M(0,2)*v[2] + M(0,3)*v[3],
                M(1,0)*v[0] + M(1,1)*v[1] + M(1,2)*v[2] + M(1,3)*v[3],
                M(2,0)*v[0] + M(2,1)*v[1] + M(2,2)*v[2] + M(2,3)*v[3],
                M(3,0)*v[0] + M(3,1)*v[1] + M(3,2)*v[2] + M(3,3)*v[3]);
}

/**
 * Interprets the parameters for a particular column of compute_seg.  Builds
 * the indicated column of T and P.
 */
static bool
compute_seg_col(int c,
                int rtype, PN_stdfloat t, const LVecBase4 &v,
                const LMatrix4 &B,
                const LMatrix4 &Bi,
                const LMatrix4 &G,
                const LMatrix4 &GB,
                LMatrix4 &T, LMatrix4 &P) {
  bool keep_orig = ((rtype & RT_KEEP_ORIG) != 0);

  if (parametrics_cat.is_debug()) {
    parametrics_cat.debug()
      << "Computing col " << c << " type " << (rtype & RT_BASE_TYPE)
      << " at " << t << " keep_orig = " << keep_orig
      << " v = " << v << "\n";
  }

  switch (rtype & RT_BASE_TYPE) {
    // RT_point defines the point on the curve at t.  This is the vector [ t^3
    // t^2 t^1 t^0 ].
    PN_stdfloat t_sqrd,t_cubed;

  case RT_POINT:
    t_sqrd = t*t;
    t_cubed = t_sqrd*t;
    T.set_col(c, LVecBase4(t_cubed, t_sqrd, t, 1.0f));
    if (keep_orig) {
      LVecBase4 vec(t_cubed, t_sqrd, t, 1.0f);
      LVecBase4 ov = col_mult(GB, vec);
      if (parametrics_cat.is_debug()) {
        parametrics_cat.debug()
          << "orig point = " << ov << "\n";
      }
      P.set_col(c, ov);
    } else {
      P.set_col(c, v);
    }
    break;

    // RT_tangent defines the tangent to the curve at t.  This is the vector [
    // 3t^2 2t 1 0 ].
  case RT_TANGENT:
    t_sqrd = t*t;
    T.set_col(c, LVecBase4(3.0f*t_sqrd, t+t, 1.0f, 0.0f));
    if (keep_orig) {
      LVecBase4 vec(3.0f*t_sqrd, /*2.0f*t*/t+t, 1.0f, 0.0f);
      LVecBase4 ov = col_mult(GB, vec);
      if (parametrics_cat.is_debug()) {
        parametrics_cat.debug()
          << "Matrix is:\n";
        GB.write(parametrics_cat.debug(false), 2);
        parametrics_cat.debug(false)
          << "vector is " << vec << "\n"
          << "orig tangent = " << ov << "\n";
      }
      P.set_col(c, ov);
    } else {
      P.set_col(c, v);
    }
    break;

    // RT_cv defines the cth control point.  This is the cth column vector
    // from Bi.
  case RT_CV:
    T.set_col(c, Bi.get_col(c));
    if (keep_orig) {
      if (parametrics_cat.is_debug()) {
        parametrics_cat.debug()
          << "orig CV = " << G.get_col(c) << "\n";
      }
      P.set_col(c, G.get_col(c));
    } else {
      P.set_col(c, v);
    }
    break;

  default:
    std::cerr << "Invalid rebuild type in compute_seg\n";
    return false;
  }

  return true;
}

/**
 * Given a set of four properties of a curve segment (e.g.  four points, four
 * tangent values, four control points, or any combination), and a basis
 * matrix, computes the corresponding geometry matrix that (together with the
 * basis matrix) represents the curve that satisfies the four properties.
 *
 * The basis matrix is passed in as B, and its inverse must be precomputed and
 * passed in as Bi.
 *
 * The result is returned in the matrix G, each column of which represents the
 * cth control vertex.  If any of the four properties has RT_KEEP_ORIG set
 * (see below), G's input value is used to define the original shape of the
 * curve; otherwise, G's input value is ignored.
 *
 * Each property is defined by an rtype, which may be any of RT_POINT,
 * RT_TANGENT, or RT_CV, and may or may not be or'ed with RT_KEEP_ORIG.  The
 * meanings of the types are as follows:
 *
 * RT_POINT defines a specific point which the curve segment must pass
 * through.  t is in the range [0,1] and represents the parametric value at
 * which the curve segment will intersect the given point.  If RT_KEEP_ORIG is
 * not set, v defines the point; otherwise, v is ignored and the original
 * curve at point t defines the point.
 *
 * RT_TANGENT defines a specific tangent value which the curve segment must
 * have at point t.  As with RT_POINT, if RT_KEEP_ORIG is not set, v defines
 * the tangent; otherwise, v is ignored and the original curve defines the
 * tangent.
 *
 * RT_CV defines a specific control vertex which the curve segment must have.
 * In this case, t is ignored.  The position within the argument list
 * determines which control vertex is applicable; e.g.  rtype0 = RT_CV defines
 * control vertex 0, and rtype2 = RT_CV defines control vertex 2.  If
 * RT_KEEP_ORIG is not set, v defines the new control vertex; otherwise, the
 * control vertex is taken from G.
 *
 * The return value is true if all the parameters are sensible, or false if
 * there is some error.
 */
bool CubicCurveseg::
compute_seg(int rtype0, PN_stdfloat t0, const LVecBase4 &v0,
            int rtype1, PN_stdfloat t1, const LVecBase4 &v1,
            int rtype2, PN_stdfloat t2, const LVecBase4 &v2,
            int rtype3, PN_stdfloat t3, const LVecBase4 &v3,
            const LMatrix4 &B,
            const LMatrix4 &Bi,
            LMatrix4 &G) {

  // We can define a cubic curve segment given four arbitrary properties of
  // the segment: any point along the curve, any tangent along the curve, any
  // control point.  Given any four such properties, a single cubic curve
  // segment is defined.

  // For a given cubic curve segment so defined, and given a basis matrix B,
  // we can define the four control vertices that represent the segment with
  // the basis matrix.  That is, we can define the matrix G such that G * B *
  // tc, where tc is [ t^3 t^2 t^1 t^0 ] for t in [ 0..1 ], represents the
  // point on the curve segment corresponding to t.

  // First, we build a matrix T, such that each of the four columns of T
  // contains the vector that would compute the corresponding property.  We
  // also build a corresponding matrix P, such that each of its columns
  // contains the vector that is the solution of the corresponding column in
  // T.

  LMatrix4 T, P, GB;

  // GB is G * B, but we only need to compute this if any of the columns wants
  // the value from the original G.
  if ((rtype0 | rtype1 | rtype2 | rtype3) & RT_KEEP_ORIG) {
    GB = G * B;
  }

  if (! (compute_seg_col(0, rtype0, t0, v0, B, Bi, G, GB, T, P) &&
         compute_seg_col(1, rtype1, t1, v1, B, Bi, G, GB, T, P) &&
         compute_seg_col(2, rtype2, t2, v2, B, Bi, G, GB, T, P) &&
         compute_seg_col(3, rtype3, t3, v3, B, Bi, G, GB, T, P))) {
    return false;
  }

  LMatrix4 Ti;
  Ti = invert(T);

  // Now we have T and P such that P represents the solution of T, when T is
  // applied to the geometry and basis matrices.  That is, each column of P
  // represents the solution computed by the corresponding column of T.  P = G
  // * B * T.

  // We simply solve for G and get G = P * T^(-1) * B^(-1).

  G = P * Ti * Bi;

  return true;
}

/**
 * Initializes the factory for reading these things from Bam files.
 */
void CubicCurveseg::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_CubicCurveseg);
}

/**
 * Factory method to generate an object of this type.
 */
TypedWritable *CubicCurveseg::
make_CubicCurveseg(const FactoryParams &params) {
  CubicCurveseg *me = new CubicCurveseg;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void CubicCurveseg::
write_datagram(BamWriter *manager, Datagram &me) {
  ParametricCurve::write_datagram(manager, me);

  Bx.write_datagram(me);
  By.write_datagram(me);
  Bz.write_datagram(me);
  Bw.write_datagram(me);
  me.add_bool(rational);
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void CubicCurveseg::
fillin(DatagramIterator &scan, BamReader *manager) {
  ParametricCurve::fillin(scan, manager);

  Bx.read_datagram(scan);
  By.read_datagram(scan);
  Bz.read_datagram(scan);
  Bw.read_datagram(scan);
  rational = scan.get_bool();
}
