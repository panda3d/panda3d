// Filename: lmatrix3_src.h
// Created by:  drose (29Jan99)
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

////////////////////////////////////////////////////////////////////
//       Class : LMatrix3
// Description : This is a 3-by-3 transform matrix.  It typically will
//               represent either a rotation-and-scale (no
//               translation) matrix in 3-d, or a full affine matrix
//               (rotation, scale, translation) in 2-d, e.g. for a
//               texture matrix.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LMatrix3) {
PUBLISHED:
  typedef const FLOATTYPE *iterator;
  typedef const FLOATTYPE *const_iterator;

  INLINE_LINMATH FLOATNAME(LMatrix3)();
  INLINE_LINMATH FLOATNAME(LMatrix3)(const FLOATNAME(LMatrix3) &other);
  INLINE_LINMATH FLOATNAME(LMatrix3) &operator = (const FLOATNAME(LMatrix3) &other);
  INLINE_LINMATH FLOATNAME(LMatrix3) &operator = (FLOATTYPE fill_value);
  INLINE_LINMATH FLOATNAME(LMatrix3)(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02,
                                     FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12,
                                     FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22);

  void fill(FLOATTYPE fill_value);
  INLINE_LINMATH void set(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02,
                          FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12,
                          FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22);

  INLINE_LINMATH void set_row(int row, const FLOATNAME(LVecBase3) &v);
  INLINE_LINMATH void set_col(int col, const FLOATNAME(LVecBase3) &v);

  INLINE_LINMATH void set_row(int row, const FLOATNAME(LVecBase2) &v);
  INLINE_LINMATH void set_col(int col, const FLOATNAME(LVecBase2) &v);

  INLINE_LINMATH FLOATNAME(LVecBase3) get_row(int row) const;
  INLINE_LINMATH FLOATNAME(LVecBase3) get_col(int col) const;

  INLINE_LINMATH FLOATNAME(LVecBase2) get_row2(int row) const;
  INLINE_LINMATH FLOATNAME(LVecBase2) get_col2(int col) const;

  // these versions inline better
  INLINE_LINMATH void get_row(FLOATNAME(LVecBase3) &result_vec, int row) const;

  INLINE_LINMATH FLOATTYPE &operator () (int row, int col);
  INLINE_LINMATH FLOATTYPE operator () (int row, int col) const;

  INLINE_LINMATH bool is_nan() const;

  INLINE_LINMATH FLOATTYPE get_cell(int row, int col) const;
  INLINE_LINMATH void set_cell(int row, int col, FLOATTYPE value);

  INLINE_LINMATH const FLOATTYPE *get_data() const;
  INLINE_LINMATH int get_num_components() const;

public:
  INLINE_LINMATH iterator begin();
  INLINE_LINMATH iterator end();

  INLINE_LINMATH const_iterator begin() const;
  INLINE_LINMATH const_iterator end() const;

PUBLISHED:
  INLINE_LINMATH bool operator < (const FLOATNAME(LMatrix3) &other) const;
  INLINE_LINMATH bool operator == (const FLOATNAME(LMatrix3) &other) const;
  INLINE_LINMATH bool operator != (const FLOATNAME(LMatrix3) &other) const;

  INLINE_LINMATH int compare_to(const FLOATNAME(LMatrix3) &other) const;
  int compare_to(const FLOATNAME(LMatrix3) &other, FLOATTYPE threshold) const;

  INLINE_LINMATH FLOATNAME(LVecBase3)
  xform(const FLOATNAME(LVecBase3) &v) const;

  INLINE_LINMATH FLOATNAME(LVecBase2)
  xform_point(const FLOATNAME(LVecBase2) &v) const;

  INLINE_LINMATH FLOATNAME(LVecBase2)
  xform_vec(const FLOATNAME(LVecBase2) &v) const;

  // this = other1 * other2
  INLINE_LINMATH void multiply(const FLOATNAME(LMatrix3) &other1, const FLOATNAME(LMatrix3) &other2);

  INLINE_LINMATH FLOATNAME(LMatrix3) operator * (const FLOATNAME(LMatrix3) &other) const;
  INLINE_LINMATH FLOATNAME(LMatrix3) operator * (FLOATTYPE scalar) const;
  INLINE_LINMATH FLOATNAME(LMatrix3) operator / (FLOATTYPE scalar) const;

  // this = scale_mat(scale_vector) * other_mat, efficiently
  INLINE_LINMATH void scale_multiply(const FLOATNAME(LVecBase3) &scale_vector,const FLOATNAME(LMatrix3) &other_mat);

  INLINE_LINMATH FLOATNAME(LMatrix3) &operator += (const FLOATNAME(LMatrix3) &other);
  INLINE_LINMATH FLOATNAME(LMatrix3) &operator -= (const FLOATNAME(LMatrix3) &other);

  INLINE_LINMATH FLOATNAME(LMatrix3) &operator *= (const FLOATNAME(LMatrix3) &other);

  INLINE_LINMATH FLOATNAME(LMatrix3) &operator *= (FLOATTYPE scalar);
  INLINE_LINMATH FLOATNAME(LMatrix3) &operator /= (FLOATTYPE scalar);

  INLINE_LINMATH FLOATTYPE determinant() const;

  INLINE_LINMATH void transpose_from(const FLOATNAME(LMatrix3) &other);
  INLINE_LINMATH void transpose_in_place();

  INLINE_LINMATH bool invert_from(const FLOATNAME(LMatrix3) &other);
  INLINE_LINMATH bool invert_in_place();

  static INLINE_LINMATH const FLOATNAME(LMatrix3) &ident_mat();

  // A 3x3 matrix is likely to be used for one of two purposes.  In
  // 2-d coordinate space (e.g. texture or surface coordinates), it
  // can contain a full affine transform, with scale, rotate,
  // translate.  In 3-d coordinate space, it can contain only scale
  // and/or rotate; e.g., the upper 3x3 rectangle of a full 4x4
  // matrix.

  // The following named constructors return 3x3 matrices suitable for
  // affine transforms in 2-d coordinate space.

  static INLINE_LINMATH FLOATNAME(LMatrix3)
    translate_mat(const FLOATNAME(LVecBase2) &trans);
  static INLINE_LINMATH FLOATNAME(LMatrix3)
    translate_mat(FLOATTYPE tx, FLOATTYPE ty);
  static INLINE_LINMATH FLOATNAME(LMatrix3)
    rotate_mat(FLOATTYPE angle);
  static INLINE_LINMATH FLOATNAME(LMatrix3)
    scale_mat(const FLOATNAME(LVecBase2) &scale);
  static INLINE_LINMATH FLOATNAME(LMatrix3)
    scale_mat(FLOATTYPE sx, FLOATTYPE sy);

  // The following named constructors return 3x3 matrices suitable for
  // scale/rotate transforms in 3-d coordinate space.
  static INLINE_LINMATH FLOATNAME(LMatrix3)
    rotate_mat(FLOATTYPE angle,
               FLOATNAME(LVecBase3) axis,
               CoordinateSystem cs = CS_default);
  static INLINE_LINMATH FLOATNAME(LMatrix3)
    rotate_mat_normaxis(FLOATTYPE angle,
                        const FLOATNAME(LVecBase3) &axis,
                        CoordinateSystem cs = CS_default);

  static INLINE_LINMATH FLOATNAME(LMatrix3)
    scale_mat(const FLOATNAME(LVecBase3) &scale);
  static INLINE_LINMATH FLOATNAME(LMatrix3)
    scale_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz);

  static INLINE_LINMATH FLOATNAME(LMatrix3)
    shear_mat(const FLOATNAME(LVecBase3) &shear);
  static INLINE_LINMATH FLOATNAME(LMatrix3)
    shear_mat(FLOATTYPE shxy, FLOATTYPE shxz, FLOATTYPE shyz);

  static const FLOATNAME(LMatrix3) &convert_mat(CoordinateSystem from,
                                                CoordinateSystem to);

  // We don't have a scale_mat() that takes a single uniform scale
  // parameter, because it would be ambiguous whether we mean a 2-d or
  // a 3-d scale.

  bool almost_equal(const FLOATNAME(LMatrix3) &other,
                    FLOATTYPE threshold) const;

  INLINE_LINMATH bool almost_equal(const FLOATNAME(LMatrix3) &other) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

public:
  INLINE_LINMATH void generate_hash(ChecksumHashGenerator &hashgen) const;
  void generate_hash(ChecksumHashGenerator &hashgen, FLOATTYPE threshold) const;

public:
  union {
    struct {
      FLOATTYPE  _00, _01, _02;
      FLOATTYPE  _10, _11, _12;
      FLOATTYPE  _20, _21, _22;
    } m;
    
    FLOATTYPE data[3 * 3];
  } _m;

private:
  INLINE_LINMATH FLOATTYPE mult_cel(const FLOATNAME(LMatrix3) &other, int x, int y) const;
  INLINE_LINMATH FLOATTYPE det2(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e10, FLOATTYPE e11) const;

  static const FLOATNAME(LMatrix3) _ident_mat;
  static const FLOATNAME(LMatrix3) _y_to_z_up_mat;
  static const FLOATNAME(LMatrix3) _z_to_y_up_mat;
  static const FLOATNAME(LMatrix3) _flip_y_mat;
  static const FLOATNAME(LMatrix3) _flip_z_mat;
  static const FLOATNAME(LMatrix3) _lz_to_ry_mat;
  static const FLOATNAME(LMatrix3) _ly_to_rz_mat;

  //Functionality for reading and writing from/to a binary source
public:
  void write_datagram(Datagram& destination) const;
  void read_datagram(DatagramIterator& scan);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};


INLINE_LINMATH ostream &operator << (ostream &out, const FLOATNAME(LMatrix3) &mat) {
  mat.output(out);
  return out;
}

INLINE_LINMATH FLOATNAME(LMatrix3) transpose(const FLOATNAME(LMatrix3) &a);
INLINE_LINMATH FLOATNAME(LMatrix3) invert(const FLOATNAME(LMatrix3) &a);

#include "lmatrix3_src.I"
