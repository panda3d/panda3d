
TypeHandle FLOATNAME(LQuaternion)::_type_handle;

const FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::_ident_quat =
  FLOATNAME(LQuaternion)(1.0, 0.0, 0.0, 0.0);

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
  FLOATTYPE N = (_data[0] * _data[0]) + (_data[1] * _data[1]) + (_data[2] * _data[2]) + (_data[3] * _data[3]);
  FLOATTYPE s = (N == 0.) ? 0. : (2. / N);
  FLOATTYPE xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

  xs = _data[1] * s;   ys = _data[2] * s;   zs = _data[3] * s;
  wx = _data[0] * xs;  wy = _data[0] * ys;  wz = _data[0] * zs;
  xx = _data[1] * xs;  xy = _data[1] * ys;  xz = _data[1] * zs;
  yy = _data[2] * ys;  yz = _data[2] * zs;  zz = _data[3] * zs;

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
  FLOATTYPE N = (_data[0] * _data[0]) + (_data[1] * _data[1]) + (_data[2] * _data[2]) + (_data[3] * _data[3]);
  FLOATTYPE s = (N == 0.) ? 0. : (2. / N);
  FLOATTYPE xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

  xs = _data[1] * s;   ys = _data[2] * s;   zs = _data[3] * s;
  wx = _data[0] * xs;  wy = _data[0] * ys;  wz = _data[0] * zs;
  xx = _data[1] * xs;  xy = _data[1] * ys;  xz = _data[1] * zs;
  yy = _data[2] * ys;  yz = _data[2] * zs;  zz = _data[3] * zs;

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
  FLOATTYPE N = (_data[0] * _data[0]) + (_data[1] * _data[1]) + (_data[2] * _data[2]) + (_data[3] * _data[3]);
  FLOATTYPE s = (N == 0.) ? 0. : (2. / N);
  FLOATTYPE xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz, c1, c2, c3, c4;
  FLOATTYPE cr, sr, cp, sp, ch, sh;

  xs = _data[1] * s;   ys = _data[2] * s;   zs = _data[3] * s;
  wx = _data[0] * xs;  wy = _data[0] * ys;  wz = _data[0] * zs;
  xx = _data[1] * xs;  xy = _data[1] * ys;  xz = _data[1] * zs;
  yy = _data[2] * ys;  yz = _data[2] * zs;  zz = _data[3] * zs;
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
//  Description: Do-While Jones.
////////////////////////////////////////////////////////////////////
void FLOATNAME(LQuaternion)::
set_from_matrix(const FLOATNAME(LMatrix3) &m) {
  FLOATTYPE m00 = m.get_cell(0, 0);
  FLOATTYPE m01 = m.get_cell(0, 1);
  FLOATTYPE m02 = m.get_cell(0, 2);
  FLOATTYPE m10 = m.get_cell(1, 0);
  FLOATTYPE m11 = m.get_cell(1, 1);
  FLOATTYPE m12 = m.get_cell(1, 2);
  FLOATTYPE m20 = m.get_cell(2, 0);
  FLOATTYPE m21 = m.get_cell(2, 1);
  FLOATTYPE m22 = m.get_cell(2, 2);

  FLOATTYPE T = m00 + m11 + m22 + 1.;

  if (T > 0.) {
    // the easy case
    FLOATTYPE S = 0.5 / csqrt(T);
    _data[0] = 0.25 / S;
    _data[1] = (m21 - m12) * S;
    _data[2] = (m02 - m20) * S;
    _data[3] = (m10 - m01) * S;
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

    FLOATTYPE S;

    switch (c) {
    case 0:
      S = csqrt(1. + m00 - m11 - m22) * 2.;
      _data[0] = (m12 + m21) / S;
      _data[1] = 0.5 / S;
      _data[2] = (m01 + m10) / S;
      _data[3] = (m02 + m20) / S;
      break;
    case 1:
      S = csqrt(1. + m11 - m00 - m22) * 2.;
      _data[0] = (m02 + m20) / S;
      _data[1] = (m01 + m10) / S;
      _data[2] = 0.5 / S;
      _data[3] = (m12 + m21) / S;
      break;
    case 2:
      S = csqrt(1. + m22 - m00 - m11) * 2.;
      _data[0] = (m01 + m10) / S;
      _data[1] = (m02 + m20) / S;
      _data[2] = (m12 + m21) / S;
      _data[3] = 0.5 / S;
      break;
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
