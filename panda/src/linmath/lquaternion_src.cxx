// Filename: lquaternion_src.cxx
// Created by:  
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

TypeHandle FLOATNAME(LQuaternion)::_type_handle;

const FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::_ident_quat =
  FLOATNAME(LQuaternion)(1.0f, 0.0f, 0.0f, 0.0f);

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::pure_imaginary_quat
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::
pure_imaginary(const FLOATNAME(LVector3) &v) {
  return FLOATNAME(LQuaternion)(0, v[0], v[1], v[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: extract (LMatrix3)
//       Access: public
//  Description: Do-While Jones paper from cary.
////////////////////////////////////////////////////////////////////
void FLOATNAME(LQuaternion)::
extract_to_matrix(FLOATNAME(LMatrix3) &m) const {
  FLOATTYPE N = (_v.data[0] * _v.data[0]) + (_v.data[1] * _v.data[1]) + (_v.data[2] * _v.data[2]) + (_v.data[3] * _v.data[3]);
  FLOATTYPE s = (N == 0.) ? 0. : (2. / N);
  FLOATTYPE xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

  xs = _v.data[1] * s;   ys = _v.data[2] * s;   zs = _v.data[3] * s;
  wx = _v.data[0] * xs;  wy = _v.data[0] * ys;  wz = _v.data[0] * zs;
  xx = _v.data[1] * xs;  xy = _v.data[1] * ys;  xz = _v.data[1] * zs;
  yy = _v.data[2] * ys;  yz = _v.data[2] * zs;  zz = _v.data[3] * zs;

  m = FLOATNAME(LMatrix3)((1. - (yy + zz)), (xy - wz), (xz + wy),
                          (xy + wz), (1. - (xx + zz)), (yz - wx),
                          (xz - wy), (yz + wx), (1. - (xx + yy)));
}

////////////////////////////////////////////////////////////////////
//     Function: extract (LMatrix4)
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LQuaternion)::
extract_to_matrix(FLOATNAME(LMatrix4) &m) const {
  FLOATTYPE N = (_v.data[0] * _v.data[0]) + (_v.data[1] * _v.data[1]) + (_v.data[2] * _v.data[2]) + (_v.data[3] * _v.data[3]);
  FLOATTYPE s = (N == 0.) ? 0. : (2. / N);
  FLOATTYPE xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

  xs = _v.data[1] * s;   ys = _v.data[2] * s;   zs = _v.data[3] * s;
  wx = _v.data[0] * xs;  wy = _v.data[0] * ys;  wz = _v.data[0] * zs;
  xx = _v.data[1] * xs;  xy = _v.data[1] * ys;  xz = _v.data[1] * zs;
  yy = _v.data[2] * ys;  yz = _v.data[2] * zs;  zz = _v.data[3] * zs;

  m = FLOATNAME(LMatrix4)((1. - (yy + zz)), (xy - wz), (xz + wy), 0.,
                          (xy + wz), (1. - (xx + zz)), (yz - wx), 0.,
                          (xz - wy), (yz + wx), (1. - (xx + yy)), 0.,
                          0., 0., 0., 1.);
}

////////////////////////////////////////////////////////////////////
//     Function: set_hpr
//       Access: public
//  Description: Sets the quaternion as the unit quaternion that
//               is equivalent to these Euler angles.
//               (from Real-time Rendering, p.49)
////////////////////////////////////////////////////////////////////
void FLOATNAME(LQuaternion)::
set_hpr(const FLOATNAME(LVecBase3) &hpr) {
  FLOATNAME(LQuaternion) quat_h, quat_p, quat_r;

  FLOATNAME(LVector3) v = FLOATNAME(LVector3)::up();
  FLOATTYPE a = deg_2_rad(hpr[0] * 0.5);
  FLOATTYPE s,c;

  csincos(a,&s,&c);
  quat_h.set(c, v[0] * s, v[1] * s, v[2] * s);
  v = FLOATNAME(LVector3)::right();
  a = deg_2_rad(hpr[1] * 0.5);
  csincos(a,&s,&c);
  s = csin(a);
  quat_p.set(c, v[0] * s, v[1] * s, v[2] * s);
  v = FLOATNAME(LVector3)::forward();
  a = deg_2_rad(hpr[2] * 0.5);
  csincos(a,&s,&c);
  quat_r.set(c, v[0] * s, v[1] * s, v[2] * s);

  (*this) = quat_h * quat_p * quat_r;
}

////////////////////////////////////////////////////////////////////
//     Function: get_hpr
//       Access: public
//  Description: Extracts the equivalent Euler angles from the unit
//               quaternion.
////////////////////////////////////////////////////////////////////
FLOATNAME(LVecBase3) FLOATNAME(LQuaternion)::
get_hpr() const {
  FLOATTYPE heading, pitch, roll;
  FLOATTYPE N = (_v.data[0] * _v.data[0]) + (_v.data[1] * _v.data[1]) + (_v.data[2] * _v.data[2]) + (_v.data[3] * _v.data[3]);
  FLOATTYPE s = (N == 0.) ? 0. : (2. / N);
  FLOATTYPE xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz, c1, c2, c3, c4;
  FLOATTYPE cr, sr, cp, sp, ch, sh;

  xs = _v.data[1] * s;   ys = _v.data[2] * s;   zs = _v.data[3] * s;
  wx = _v.data[0] * xs;  wy = _v.data[0] * ys;  wz = _v.data[0] * zs;
  xx = _v.data[1] * xs;  xy = _v.data[1] * ys;  xz = _v.data[1] * zs;
  yy = _v.data[2] * ys;  yz = _v.data[2] * zs;  zz = _v.data[3] * zs;
  c1 = xz - wy;
  c2 = 1. - (xx + yy);
  c3 = 1. - (yy + zz);
  c4 = xy + wz;

  if (c1 == 0.) {  // (roll = 0 or 180) or (pitch = +/- 90
    if (c2 >= 0.) {
      roll = 0.;
      ch = c3;
      sh = c4;
      cp = c2;
    } else {
      roll = 180.;
      ch = -c3;
      sh = -c4;
      cp = -c2;
    }
  } else {
    // this should work all the time, but the above saves some trig operations
    roll = catan2(-c1, c2);
    csincos(roll,&sr,&cr);
    roll = rad_2_deg(roll);
    ch = (cr * c3) + (sr * (xz + wy));
    sh = (cr * c4) + (sr * (yz - wx));
    cp = (cr * c2) - (sr * c1);
  }
  sp = yz + wx;
  heading = rad_2_deg(catan2(sh, ch));
  pitch = rad_2_deg(catan2(sp, cp));

  return FLOATNAME(LVecBase3)(heading, pitch, roll);
}

////////////////////////////////////////////////////////////////////
//     Function: set_from_matrix
//       Access: public
//  Description: Sets the quaternion according to the rotation
//               represented by the matrix.  Originally we tried an
//               algorithm presented by Do-While Jones, but that
//               turned out to be broken.
////////////////////////////////////////////////////////////////////
void FLOATNAME(LQuaternion)::
set_from_matrix(const FLOATNAME(LMatrix3) &m) {
  FLOATTYPE m00 = m(0, 0);
  FLOATTYPE m01 = m(0, 1);
  FLOATTYPE m02 = m(0, 2);
  FLOATTYPE m10 = m(1, 0);
  FLOATTYPE m11 = m(1, 1);
  FLOATTYPE m12 = m(1, 2);
  FLOATTYPE m20 = m(2, 0);
  FLOATTYPE m21 = m(2, 1);
  FLOATTYPE m22 = m(2, 2);

  FLOATTYPE T = m00 + m11 + m22;

  if (T > 0.0f) {
    // The easy case.
    FLOATTYPE S = csqrt(T + 1.0f);
    _v.data[0] = S * 0.5f;
    S = 0.5f / S;
    _v.data[1] = (m21 - m12) * S;
    _v.data[2] = (m02 - m20) * S;
    _v.data[3] = (m10 - m01) * S;

  } else {
    // The harder case.  First, figure out which column to take as
    // root.  We'll choose the largest so that we get the greatest
    // precision.

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
      _v.data[1] = S * 0.5f;
      S = 0.5f / S;
      _v.data[2] = (m10 + m01) * S;
      _v.data[3] = (m20 + m02) * S;
      _v.data[0] = (m21 - m12) * S;

    } else if (m11 > m22) {
      // m11 is larger than m00 and m22.
      FLOATTYPE S = 1.0f + m11 - (m22 + m00);
      nassertv(S > 0.0f);
      S = csqrt(S);
      _v.data[2] = S * 0.5f;
      S = 0.5f / S;
      _v.data[3] = (m21 + m12) * S;
      _v.data[1] = (m01 + m10) * S;
      _v.data[0] = (m02 - m20) * S;

    } else {
      // m22 is larger than m00 and m11.
      FLOATTYPE S = 1.0f + m22 - (m00 + m11);
      nassertv(S > 0.0f);
      S = csqrt(S);
      _v.data[3] = S * 0.5f;
      S = 0.5f / S;
      _v.data[1] = (m02 + m20) * S;
      _v.data[2] = (m12 + m21) * S;
      _v.data[0] = (m10 - m01) * S;
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
    string name = "LQuaternion";
    name += FLOATTOKEN;
    register_type(_type_handle, name,
                  FLOATNAME(LVecBase4)::get_class_type());
  }
}
