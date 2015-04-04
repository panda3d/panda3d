// Filename: lquaternion_src.cxx
// Created by:  
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

#include "config_linmath.h"
#include "lmatrix.h"
#include "luse.h"

TypeHandle FLOATNAME(LQuaternion)::_type_handle;

const FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::_ident_quat =
  FLOATNAME(LQuaternion)(1.0f, 0.0f, 0.0f, 0.0f);

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::pure_imaginary_quat
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::
pure_imaginary(const FLOATNAME(LVector3) &v) {
  return FLOATNAME(LQuaternion)(0, v[0], v[1], v[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::extract_to_matrix (LMatrix3)
//       Access: Public
//  Description: Based on the quat lib from VRPN.
////////////////////////////////////////////////////////////////////
void FLOATNAME(LQuaternion)::
extract_to_matrix(FLOATNAME(LMatrix3) &m) const {
  FLOATTYPE N = this->dot(*this);
  FLOATTYPE s = (N == 0.0f) ? 0.0f : (2.0f / N);
  FLOATTYPE xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

  xs = _v(1) * s;   ys = _v(2) * s;   zs = _v(3) * s;
  wx = _v(0) * xs;  wy = _v(0) * ys;  wz = _v(0) * zs;
  xx = _v(1) * xs;  xy = _v(1) * ys;  xz = _v(1) * zs;
  yy = _v(2) * ys;  yz = _v(2) * zs;  zz = _v(3) * zs;

  m.set((1.0f - (yy + zz)), (xy + wz), (xz - wy),
        (xy - wz), (1.0f - (xx + zz)), (yz + wx),
        (xz + wy), (yz - wx), (1.0f - (xx + yy)));
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::extract_to_matrix (LMatrix4)
//       Access: Public
//  Description: Based on the quat lib from VRPN.
////////////////////////////////////////////////////////////////////
void FLOATNAME(LQuaternion)::
extract_to_matrix(FLOATNAME(LMatrix4) &m) const {
  FLOATTYPE N = this->dot(*this);
  FLOATTYPE s = (N == 0.0f) ? 0.0f : (2.0f / N);
  FLOATTYPE xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

  xs = _v(1) * s;   ys = _v(2) * s;   zs = _v(3) * s;
  wx = _v(0) * xs;  wy = _v(0) * ys;  wz = _v(0) * zs;
  xx = _v(1) * xs;  xy = _v(1) * ys;  xz = _v(1) * zs;
  yy = _v(2) * ys;  yz = _v(2) * zs;  zz = _v(3) * zs;

  m.set((1.0f - (yy + zz)), (xy + wz), (xz - wy), 0.0f,
        (xy - wz), (1.0f - (xx + zz)), (yz + wx), 0.0f,
        (xz + wy), (yz - wx), (1.0f - (xx + yy)), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::set_hpr
//       Access: public
//  Description: Sets the quaternion as the unit quaternion that
//               is equivalent to these Euler angles.
//               (from Real-time Rendering, p.49)
////////////////////////////////////////////////////////////////////
void FLOATNAME(LQuaternion)::
set_hpr(const FLOATNAME(LVecBase3) &hpr, CoordinateSystem cs) {
  FLOATNAME(LQuaternion) quat_h, quat_p, quat_r;

  FLOATNAME(LVector3) v;
  FLOATTYPE a, s, c;

  v = FLOATNAME(LVector3)::up(cs);
  a = deg_2_rad(hpr[0] * 0.5f);
  csincos(a, &s, &c);
  quat_h.set(c, v[0] * s, v[1] * s, v[2] * s);
  v = FLOATNAME(LVector3)::right(cs);
  a = deg_2_rad(hpr[1] * 0.5f);
  csincos(a, &s, &c);
  s = csin(a);
  quat_p.set(c, v[0] * s, v[1] * s, v[2] * s);
  v = FLOATNAME(LVector3)::forward(cs);
  a = deg_2_rad(hpr[2] * 0.5f);
  csincos(a, &s, &c);
  quat_r.set(c, v[0] * s, v[1] * s, v[2] * s);

  if (is_right_handed(cs)) {
    (*this) = quat_r * quat_p * quat_h;
  } else {
    (*this) = invert(quat_h * quat_p * quat_r);
  }

  if (!temp_hpr_fix) {
    // Compute the old, broken hpr.
    (*this) = quat_p * quat_h * invert(quat_r);
  }

#ifndef NDEBUG
  if (paranoid_hpr_quat) {
    FLOATNAME(LMatrix3) mat;
    compose_matrix(mat, FLOATNAME(LVecBase3)(1.0f, 1.0f, 1.0f), hpr, cs);
    FLOATNAME(LQuaternion) compare;
    compare.set_from_matrix(mat);
    if (!compare.almost_equal(*this) && !compare.almost_equal(-(*this))) {
      linmath_cat.warning()
        << "hpr-to-quat of " << hpr << " computed " << *this
        << " instead of " << compare << "\n";
      (*this) = compare;
    }
  }
#endif  // NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::get_hpr
//       Access: public
//  Description: Extracts the equivalent Euler angles from the unit
//               quaternion.
////////////////////////////////////////////////////////////////////
FLOATNAME(LVecBase3) FLOATNAME(LQuaternion)::
get_hpr(CoordinateSystem cs) const {
  if (!temp_hpr_fix) {
    // With the old, broken hpr code in place, just go through the
    // existing matrix decomposition code.  Not particularly speedy,
    // but I don't want to bother with working out how to do it
    // directly for code that hopefully won't need to last much
    // longer.
    FLOATNAME(LMatrix3) mat;
    extract_to_matrix(mat);
    FLOATNAME(LVecBase3) scale, hpr;
    decompose_matrix(mat, scale, hpr, cs);
    return hpr;
  }

  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  FLOATNAME(LVecBase3) hpr;

  if (cs == CS_zup_right) {
    FLOATTYPE N =
        (_v(0) * _v(0)) +
        (_v(1) * _v(1)) +
        (_v(2) * _v(2)) +
        (_v(3) * _v(3));
    FLOATTYPE s = (N == 0.0f) ? 0.0f : (2.0f / N);
    FLOATTYPE xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz, c1, c2, c3, c4;
    FLOATTYPE cr, sr, cp, sp, ch, sh;
    
    xs = _v(1) * s;   ys = _v(2) * s;   zs = _v(3) * s;
    wx = _v(0) * xs;  wy = _v(0) * ys;  wz = _v(0) * zs;
    xx = _v(1) * xs;  xy = _v(1) * ys;  xz = _v(1) * zs;
    yy = _v(2) * ys;  yz = _v(2) * zs;  zz = _v(3) * zs;
    c1 = xz - wy;
    c2 = 1.0f - (xx + yy);
    c3 = 1.0f - (yy + zz);
    c4 = xy + wz;
    
    if (c1 == 0.0f) {  // (roll = 0 or 180) or (pitch = +/- 90)
      if (c2 >= 0.0f) {
        hpr[2] = 0.0f;
        ch = c3;
        sh = c4;
        cp = c2;
      } else {
        hpr[2] = 180.0f;
        ch = -c3;
        sh = -c4;
        cp = -c2;
      }
    } else {
      // this should work all the time, but the above saves some trig operations
      FLOATTYPE roll = catan2(-c1, c2);
      csincos(roll, &sr, &cr);
      hpr[2] = rad_2_deg(roll);
      ch = (cr * c3) + (sr * (xz + wy));
      sh = (cr * c4) + (sr * (yz - wx));
      cp = (cr * c2) - (sr * c1);
    }
    sp = yz + wx;
    hpr[0] = rad_2_deg(catan2(sh, ch));
    hpr[1] = rad_2_deg(catan2(sp, cp));

  } else {
    // The code above implements quat-to-hpr for CS_zup_right only.
    // For other coordinate systems, someone is welcome to extend the
    // implementation; I'm going to choose the lazy path till then.
    FLOATNAME(LMatrix3) mat;
    extract_to_matrix(mat);
    FLOATNAME(LVecBase3) scale;
    decompose_matrix(mat, scale, hpr, cs);
    return hpr;
  }


#ifndef NDEBUG
  if (paranoid_hpr_quat) {
    FLOATNAME(LMatrix3) mat;
    extract_to_matrix(mat);
    FLOATNAME(LVecBase3) scale, compare_hpr;
    decompose_matrix(mat, scale, compare_hpr, cs);
    if (!compare_hpr.almost_equal(hpr)) {
      linmath_cat.warning()
        << "quat-to-hpr of " << *this << " computed " << hpr << " instead of "
        << compare_hpr << "\n";
      hpr = compare_hpr;
    }
  }
#endif  // NDEBUG

  return hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::set_from_matrix
//       Access: public
//  Description: Sets the quaternion according to the rotation
//               represented by the matrix.  Originally we tried an
//               algorithm presented by Do-While Jones, but that
//               turned out to be broken.  This is based on the quat
//               lib from UNC.
////////////////////////////////////////////////////////////////////
void FLOATNAME(LQuaternion)::
set_from_matrix(const FLOATNAME(LMatrix3) &m) {
  FLOATTYPE m00, m01, m02, m10, m11, m12, m20, m21, m22;

  m00 = m(0, 0);
  m10 = m(1, 0);
  m20 = m(2, 0);
  m01 = m(0, 1);
  m11 = m(1, 1);
  m21 = m(2, 1);
  m02 = m(0, 2);
  m12 = m(1, 2);
  m22 = m(2, 2);

  FLOATTYPE trace = m00 + m11 + m22;

  if (trace > 0.0f) {
    // The easy case.
    FLOATTYPE S = csqrt(trace + 1.0f);
    _v(0) = S * 0.5f;
    S = 0.5f / S;
    _v(1) = (m12 - m21) * S;
    _v(2) = (m20 - m02) * S;
    _v(3) = (m01 - m10) * S;

  } else {
    // The harder case.  First, figure out which column to take as
    // root.  This will be the column with the largest value.

    // It is tempting to try to compare the absolute values of the
    // diagonal values in the code below, instead of their normal,
    // signed values.  Don't do it.  We are actually maximizing the
    // value of S, which must always be positive, and is therefore
    // based on the diagonal whose actual value--not absolute
    // value--is greater than those of the other two.

    // We already know that m00 + m11 + m22 <= 0 (because we are here
    // in the harder case).

    if (m00 > m11 && m00 > m22) {
      // m00 is larger than m11 and m22.
      FLOATTYPE S = 1.0f + m00 - (m11 + m22);
      nassertv(S > 0.0f);
      S = csqrt(S);
      _v(1) = S * 0.5f;
      S = 0.5f / S;
      _v(2) = (m01 + m10) * S;
      _v(3) = (m02 + m20) * S;
      _v(0) = (m12 - m21) * S;

    } else if (m11 > m22) {
      // m11 is larger than m00 and m22.
      FLOATTYPE S = 1.0f + m11 - (m22 + m00);
      nassertv(S > 0.0f);
      S = csqrt(S);
      _v(2) = S * 0.5f;
      S = 0.5f / S;
      _v(3) = (m12 + m21) * S;
      _v(1) = (m10 + m01) * S;
      _v(0) = (m20 - m02) * S;

    } else {
      // m22 is larger than m00 and m11.
      FLOATTYPE S = 1.0f + m22 - (m00 + m11);
      nassertv(S > 0.0f);
      S = csqrt(S);
      _v(3) = S * 0.5f;
      S = 0.5f / S;
      _v(1) = (m20 + m02) * S;
      _v(2) = (m21 + m12) * S;
      _v(0) = (m01 - m10) * S;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::init_type
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LQuaternion)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LVecBase4)::init_type();
    register_type(_type_handle, FLOATNAME_STR(LQuaternion),
                  FLOATNAME(LVecBase4)::get_class_type());
  }
}
