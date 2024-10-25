/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file transform.cxx
 * @author rdb
 * @date 2024-10-17
 */

#include "transform.h"
#include "transformState.h"

TypeHandle Transform::_type_handle;

/**
 * Converts from a deprecated TransformState object to a Transform.
 */
Transform::
Transform(const TransformState *state) {
  const LMatrix4 &mat = state->get_mat();
  _columns[0] = mat.get_col(0);
  _columns[1] = mat.get_col(1);
  _columns[2] = mat.get_col(2);
}

/**
 * Makes a new Transform with the specified components.
 */
Transform Transform::
make_pos_hpr_scale_shear(const LVecBase3 &pos, const LVecBase3 &hpr,
                         LVecBase3 scale, const LVecBase3 &shear) {
  if (scale[0] == 0 || scale[1] == 0 || scale[2] == 0) {
    nassert_raise("Tried to apply scale with zero component!");
    scale = (scale[0] != 0) ? scale[0] : 1;
    scale = (scale[1] != 0) ? scale[1] : 1;
    scale = (scale[2] != 0) ? scale[2] : 1;
  }

  LMatrix3 mat;
  compose_matrix(mat, scale, shear, hpr);
  Transform result;
  result._columns[0][0] = mat[0][0];
  result._columns[0][1] = mat[1][0];
  result._columns[0][2] = mat[2][0];
  result._columns[0][3] = pos[0];
  result._columns[1][0] = mat[0][1];
  result._columns[1][1] = mat[1][1];
  result._columns[1][2] = mat[2][1];
  result._columns[1][3] = pos[1];
  result._columns[2][0] = mat[0][2];
  result._columns[2][1] = mat[1][2];
  result._columns[2][2] = mat[2][2];
  result._columns[2][3] = pos[2];
  return result;
}

/**
 * Makes a new Transform with the specified components.
 */
Transform Transform::
make_pos_quat_scale(const LVecBase3 &pos, const LQuaternion &quat,
                    LVecBase3 scale) {
  if (scale[0] == 0 || scale[1] == 0 || scale[2] == 0) {
    nassert_raise("Tried to apply scale with zero component!");
    scale = (scale[0] != 0) ? scale[0] : 1;
    scale = (scale[1] != 0) ? scale[1] : 1;
    scale = (scale[2] != 0) ? scale[2] : 1;
  }

  PN_stdfloat length_sq = quat.dot(quat);
  PN_stdfloat s = (length_sq == 0.0f) ? 0.0f : (2.0f / length_sq);
  PN_stdfloat xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;
  xs = quat._v(1) * s;   ys = quat._v(2) * s;   zs = quat._v(3) * s;
  wx = quat._v(0) * xs;  wy = quat._v(0) * ys;  wz = quat._v(0) * zs;
  xx = quat._v(1) * xs;  xy = quat._v(1) * ys;  xz = quat._v(1) * zs;
  yy = quat._v(2) * ys;  yz = quat._v(2) * zs;  zz = quat._v(3) * zs;

  Transform result;
  result.set_mat3(LMatrix3::scale_mat(scale) * LMatrix3(
    (1.0f - (yy + zz)), (xy + wz), (xz - wy),
    (xy - wz), (1.0f - (xx + zz)), (yz + wx),
    (xz + wy), (yz - wx), (1.0f - (xx + yy))));
  result.set_pos(pos);
  return result;
}

/**
 * Makes a new Transform with the specified components.
 */
Transform Transform::
make_pos_quat_scale_shear(const LVecBase3 &pos, const LQuaternion &quat,
                          LVecBase3 scale, const LVecBase3 &shear) {
  if (scale[0] == 0 || scale[1] == 0 || scale[2] == 0) {
    nassert_raise("Tried to apply scale with zero component!");
    scale = (scale[0] != 0) ? scale[0] : 1;
    scale = (scale[1] != 0) ? scale[1] : 1;
    scale = (scale[2] != 0) ? scale[2] : 1;
  }

  PN_stdfloat length_sq = quat.dot(quat);
  PN_stdfloat s = (length_sq == 0.0f) ? 0.0f : (2.0f / length_sq);
  PN_stdfloat xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;
  xs = quat._v(1) * s;   ys = quat._v(2) * s;   zs = quat._v(3) * s;
  wx = quat._v(0) * xs;  wy = quat._v(0) * ys;  wz = quat._v(0) * zs;
  xx = quat._v(1) * xs;  xy = quat._v(1) * ys;  xz = quat._v(1) * zs;
  yy = quat._v(2) * ys;  yz = quat._v(2) * zs;  zz = quat._v(3) * zs;

  Transform result;
  result.set_mat3(LMatrix3::scale_shear_mat(scale, shear) * LMatrix3(
    (1.0f - (yy + zz)), (xy + wz), (xz - wy),
    (xy - wz), (1.0f - (xx + zz)), (yz + wx),
    (xz + wy), (yz - wx), (1.0f - (xx + yy))));
  result.set_pos(pos);
  return result;
}

/**
 * Returns the rotation component of the transform as a trio of Euler angles.
 */
LVecBase3 Transform::
get_hpr() const {
  LVecBase3 scale, shear, hpr;
  decompose_matrix(get_mat3(), scale, shear, hpr);
  return hpr;
}

/**
 * Sets the rotation component of the transform as a trio of Euler angles.
 */
void Transform::
set_hpr(const LVecBase3 &hpr) {
  LVecBase3 scale, shear, prev_hpr;
  decompose_matrix(get_mat3(), scale, shear, prev_hpr);

  LMatrix3 mat;
  compose_matrix(mat, scale, shear, hpr);
  set_mat3(mat);
}

/**
 * Sets the rotation component of the transform as a quaternion.
 */
void Transform::
set_quat(const LQuaternion &quat) {
  LVecBase3 scale, shear, prev_hpr;
  decompose_matrix(get_mat3(), scale, shear, prev_hpr);

  PN_stdfloat length_sq = quat.dot(quat);
  PN_stdfloat s = (length_sq == 0.0f) ? 0.0f : (2.0f / length_sq);
  PN_stdfloat xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;
  xs = quat._v(1) * s;   ys = quat._v(2) * s;   zs = quat._v(3) * s;
  wx = quat._v(0) * xs;  wy = quat._v(0) * ys;  wz = quat._v(0) * zs;
  xx = quat._v(1) * xs;  xy = quat._v(1) * ys;  xz = quat._v(1) * zs;
  yy = quat._v(2) * ys;  yz = quat._v(2) * zs;  zz = quat._v(3) * zs;

  set_mat3(LMatrix3::scale_shear_mat(scale, shear) * LMatrix3(
      (1.0f - (yy + zz)), (xy + wz), (xz - wy),
      (xy - wz), (1.0f - (xx + zz)), (yz + wx),
      (xz + wy), (yz - wx), (1.0f - (xx + yy))));
}

/**
 * Returns the scale component of the transform.
 */
LVecBase3 Transform::
get_scale() const {
  LVecBase3 scale, shear, hpr;
  decompose_matrix(get_mat3(), scale, shear, hpr);
  return scale;
}

/**
 * Sets the scale component of the transform.
 */
void Transform::
set_scale(LVecBase3 scale) {
  if (scale[0] == 0 || scale[1] == 0 || scale[2] == 0) {
    nassert_raise("Tried to apply scale with zero component!");
    scale = (scale[0] != 0) ? scale[0] : 1;
    scale = (scale[1] != 0) ? scale[1] : 1;
    scale = (scale[2] != 0) ? scale[2] : 1;
  }
  LVecBase3 prev_scale, shear, hpr;
  decompose_matrix(get_mat3(), prev_scale, shear, hpr);

  LMatrix3 mat;
  compose_matrix(mat, scale, shear, hpr);
  set_mat3(mat);
}

/**
 * Returns the shear component of the transform.
 */
LVecBase3 Transform::
get_shear() const {
  LVecBase3 scale, shear, hpr;
  decompose_matrix(get_mat3(), scale, shear, hpr);
  return shear;
}

/**
 * Sets the shear component of the transform.
 */
void Transform::
set_shear(const LVecBase3 &shear) {
  LVecBase3 scale, prev_shear, hpr;
  decompose_matrix(get_mat3(), scale, prev_shear, hpr);

  LMatrix3 mat;
  compose_matrix(mat, scale, shear, hpr);
  set_mat3(mat);
}

/**
 *
 */
void Transform::
output(ostream &out) const {
  out << "T:";
  if (is_invalid()) {
    out << "(invalid)";
  }
  else if (is_identity()) {
    out << "(identity)";
  }
  /*else if (has_components()) {
    bool output_hpr = !get_hpr().almost_equal(LVecBase3(0.0f, 0.0f, 0.0f));

    if (!components_given()) {
      // A leading "m" indicates the transform was described as a full matrix,
      // and we are decomposing it for the benefit of the user.
      out << "m";

    } else if (output_hpr && quat_given()) {
      // A leading "q" indicates that the pos, scale, and shear are exactly as
      // specified, but the rotation was described as a quaternion, and we are
      // decomposing that to hpr for the benefit of the user.
      out << "q";
    }

    char lead = '(';
    if (is_2d()) {
      if (!get_pos2d().almost_equal(LVecBase2(0.0f, 0.0f))) {
        out << lead << "pos " << get_pos2d();
        lead = ' ';
      }
      if (output_hpr) {
        out << lead << "rotate " << get_rotate2d();
        lead = ' ';
      }
      if (!get_scale2d().almost_equal(LVecBase2(1.0f, 1.0f))) {
        if (has_uniform_scale()) {
          out << lead << "scale " << get_uniform_scale();
          lead = ' ';
        } else {
          out << lead << "scale " << get_scale2d();
          lead = ' ';
        }
      }
      if (has_nonzero_shear()) {
        out << lead << "shear " << get_shear2d();
        lead = ' ';
      }
    } else {
      if (!get_pos().almost_equal(LVecBase3(0.0f, 0.0f, 0.0f))) {
        out << lead << "pos " << get_pos();
        lead = ' ';
      }
      if (output_hpr) {
        out << lead << "hpr " << get_hpr();
        lead = ' ';
      }
      if (!get_scale().almost_equal(LVecBase3(1.0f, 1.0f, 1.0f))) {
        if (has_uniform_scale()) {
          out << lead << "scale " << get_uniform_scale();
          lead = ' ';
        } else {
          out << lead << "scale " << get_scale();
          lead = ' ';
        }
      }
      if (has_nonzero_shear()) {
        out << lead << "shear " << get_shear();
        lead = ' ';
      }
    }
    if (lead == '(') {
      out << "(almost identity)";
    } else {
      out << ")";
    }

  } else {
    if (is_2d()) {
      out << get_mat3();
    } else {
      out << get_mat();
    }
  }*/
}

/**
 *
 */
void Transform::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}
