
TypeHandle FLOATNAME(LQuaternionBase)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::Destructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

FLOATNAME(LQuaternionBase)::
~FLOATNAME(LQuaternionBase)() {
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::pure_imaginary_quat
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

FLOATNAME(LQuaternionBase) FLOATNAME(LQuaternionBase)::
pure_imaginary(const FLOATNAME(LVector3) &v) {
  return FLOATNAME(LQuaternionBase)(0, v[0], v[1], v[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: extract (LMatrix3)
//       Access: public
//  Description: Do-While Jones paper from cary.
////////////////////////////////////////////////////////////////////

void FLOATNAME(LQuaternionBase)::
extract_to_matrix(FLOATNAME(LMatrix3) &m) const {
  FLOATTYPE1 N = (_r * _r) + (_i * _i) + (_j * _j) + (_k * _k);
  FLOATTYPE1 s = (N == 0.) ? 0. : (2. / N);
  FLOATTYPE1 xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

  xs = _i * s;   ys = _j * s;   zs = _k * s;
  wx = _r * xs;  wy = _r * ys;  wz = _r * zs;
  xx = _i * xs;  xy = _i * ys;  xz = _i * zs;
  yy = _j * ys;  yz = _j * zs;  zz = _k * zs;

  m = FLOATNAME(LMatrix3)((1. - (yy + zz)), (xy - wz), (xz + wy),
			(xy + wz), (1. - (xx + zz)), (yz - wx),
			(xz - wy), (yz + wx), (1. - (xx + yy)));
}

////////////////////////////////////////////////////////////////////
//     Function: extract (LMatrix4)
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

void FLOATNAME(LQuaternionBase)::
extract_to_matrix(FLOATNAME(LMatrix4) &m) const {
  FLOATTYPE1 N = (_r * _r) + (_i * _i) + (_j * _j) + (_k * _k);
  FLOATTYPE1 s = (N == 0.) ? 0. : (2. / N);
  FLOATTYPE1 xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

  xs = _i * s;   ys = _j * s;   zs = _k * s;
  wx = _r * xs;  wy = _r * ys;  wz = _r * zs;
  xx = _i * xs;  xy = _i * ys;  xz = _i * zs;
  yy = _j * ys;  yz = _j * zs;  zz = _k * zs;

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

void FLOATNAME(LQuaternionBase)::
set_hpr(const FLOATNAME(LVecBase3) &hpr) {
  FLOATNAME(LQuaternionBase) quat_h, quat_p, quat_r;

  FLOATNAME(LVector3) v = FLOATNAME(LVector3)::up();
  FLOATTYPE1 a = deg_2_rad(hpr[0] * 0.5);
  FLOATTYPE1 s,c;

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

FLOATNAME(LVecBase3) FLOATNAME(LQuaternionBase)::
get_hpr() const {
  FLOATTYPE1 heading, pitch, roll;
  FLOATTYPE1 N = (_r * _r) + (_i * _i) + (_j * _j) + (_k * _k);
  FLOATTYPE1 s = (N == 0.) ? 0. : (2. / N);
  FLOATTYPE1 xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz, c1, c2, c3, c4;
  FLOATTYPE1 cr, sr, cp, sp, ch, sh;

  xs = _i * s;   ys = _j * s;   zs = _k * s;
  wx = _r * xs;  wy = _r * ys;  wz = _r * zs;
  xx = _i * xs;  xy = _i * ys;  xz = _i * zs;
  yy = _j * ys;  yz = _j * zs;  zz = _k * zs;
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
//     Function: set
//       Access: public
//  Description: Do-While Jones.
////////////////////////////////////////////////////////////////////

void FLOATNAME(LQuaternionBase)::
set(const FLOATNAME(LMatrix3) &m) {
  FLOATTYPE1 m00 = m.get_cell(0, 0);
  FLOATTYPE1 m01 = m.get_cell(0, 1);
  FLOATTYPE1 m02 = m.get_cell(0, 2);
  FLOATTYPE1 m10 = m.get_cell(1, 0);
  FLOATTYPE1 m11 = m.get_cell(1, 1);
  FLOATTYPE1 m12 = m.get_cell(1, 2);
  FLOATTYPE1 m20 = m.get_cell(2, 0);
  FLOATTYPE1 m21 = m.get_cell(2, 1);
  FLOATTYPE1 m22 = m.get_cell(2, 2);

  FLOATTYPE1 T = m00 + m11 + m22 + 1.;

  if (T > 0.) {
    // the easy case
    FLOATTYPE1 S = 0.5 / csqrt(T);
    _r = 0.25 / S;
    _i = (m21 - m12) * S;
    _j = (m02 - m20) * S;
    _k = (m10 - m01) * S;
  } else {
    // figure out which column to take as root
    int c = 0;
    if (cabs(m00) > cabs(m11)) {
      if (cabs(m00) > cabs(m22))
	c = 0;
      else
	c = 2;
    } else if (cabs(m11) > cabs(m22))
      c = 1;
    else
      c = 2;

    FLOATTYPE1 S;

    switch (c) {
    case 0:
      S = csqrt(1. + m00 - m11 - m22) * 2.;
      _r = (m12 + m21) / S;
      _i = 0.5 / S;
      _j = (m01 + m10) / S;
      _k = (m02 + m20) / S;
      break;
    case 1:
      S = csqrt(1. + m11 - m00 - m22) * 2.;
      _r = (m02 + m20) / S;
      _i = (m01 + m10) / S;
      _j = 0.5 / S;
      _k = (m12 + m21) / S;
      break;
    case 2:
      S = csqrt(1. + m22 - m00 - m11) * 2.;
      _r = (m01 + m10) / S;
      _i = (m02 + m20) / S;
      _j = (m12 + m21) / S;
      _k = 0.5 / S;
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::ident_quat
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

const FLOATNAME(LQuaternionBase) &FLOATNAME(LQuaternionBase)::
ident_quat(void) {
  static FLOATNAME(LQuaternionBase) q(1, 0, 0, 0);
  return q;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::init_type
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

void FLOATNAME(LQuaternionBase)::
init_type(void) {
  if (_type_handle == TypeHandle::none()) {
    do_init_type(FLOATTYPE1);
    string name = "FLOATNAME(LQuaternionBase)<" + get_type_handle(FLOATTYPE1).get_name()
                  + ">";
    register_type(_type_handle, name);
  }
}
