// Filename: lmatrix4_src.h
// Created by:  drose (15Jan99)
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
//       Class : LMatrix4
// Description : This is a 4-by-4 transform matrix.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LMatrix4) {
PUBLISHED:
  typedef const FLOATTYPE *iterator;
  typedef const FLOATTYPE *const_iterator;

  INLINE_LINMATH FLOATNAME(LMatrix4)();
  INLINE_LINMATH FLOATNAME(LMatrix4)(const FLOATNAME(LMatrix4) &other);
  INLINE_LINMATH FLOATNAME(LMatrix4) &operator = (const FLOATNAME(LMatrix4) &other);
  INLINE_LINMATH FLOATNAME(LMatrix4) &operator = (FLOATTYPE fill_value);

  INLINE_LINMATH FLOATNAME(LMatrix4)(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02, FLOATTYPE e03,
                                     FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12, FLOATTYPE e13,
                                     FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22, FLOATTYPE e23,
                                     FLOATTYPE e30, FLOATTYPE e31, FLOATTYPE e32, FLOATTYPE e33);

  // Construct a 4x4 matrix given a 3x3 rotation matrix and an optional
  // translation component.
  INLINE_LINMATH FLOATNAME(LMatrix4)(const FLOATNAME(LMatrix3) &upper3);
  INLINE_LINMATH FLOATNAME(LMatrix4)(const FLOATNAME(LMatrix3) &upper3,const FLOATNAME(LVecBase3) &trans);

  INLINE_LINMATH void fill(FLOATTYPE fill_value);
  INLINE_LINMATH void set(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02, FLOATTYPE e03,
                  FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12, FLOATTYPE e13,
                  FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22, FLOATTYPE e23,
                  FLOATTYPE e30, FLOATTYPE e31, FLOATTYPE e32, FLOATTYPE e33);

  // Get and set the upper 3x3 rotation matrix.
  INLINE_LINMATH void set_upper_3(const FLOATNAME(LMatrix3) &upper3);
  INLINE_LINMATH FLOATNAME(LMatrix3) get_upper_3() const;

  INLINE_LINMATH void set_row(int row, const FLOATNAME(LVecBase4) &v);
  INLINE_LINMATH void set_col(int col, const FLOATNAME(LVecBase4) &v);

  INLINE_LINMATH void set_row(int row, const FLOATNAME(LVecBase3) &v);
  INLINE_LINMATH void set_col(int col, const FLOATNAME(LVecBase3) &v);

  INLINE_LINMATH FLOATNAME(LVecBase4) get_row(int row) const;
  INLINE_LINMATH FLOATNAME(LVecBase4) get_col(int col) const;

  INLINE_LINMATH FLOATNAME(LVecBase3) get_row3(int row) const;

  // these versions inline better
  INLINE_LINMATH void get_row(FLOATNAME(LVecBase4) &result_vec, int row) const;
  INLINE_LINMATH void get_row3(FLOATNAME(LVecBase3) &result_vec, int row) const;

  INLINE_LINMATH FLOATNAME(LVecBase3) get_col3(int col) const;

  INLINE_LINMATH FLOATTYPE &operator () (int row, int col);
  INLINE_LINMATH FLOATTYPE operator () (int row, int col) const;

  INLINE_LINMATH bool is_nan() const;

  INLINE_LINMATH FLOATTYPE get_cell(int row, int col) const;
  INLINE_LINMATH void set_cell(int row, int col, FLOATTYPE value);

  INLINE_LINMATH const FLOATTYPE *get_data() const;
  INLINE_LINMATH int get_num_components() const;

  INLINE_LINMATH iterator begin();
  INLINE_LINMATH iterator end();

  INLINE_LINMATH const_iterator begin() const;
  INLINE_LINMATH const_iterator end() const;

  INLINE_LINMATH bool operator < (const FLOATNAME(LMatrix4) &other) const;
  INLINE_LINMATH bool operator == (const FLOATNAME(LMatrix4) &other) const;
  INLINE_LINMATH bool operator != (const FLOATNAME(LMatrix4) &other) const;

  INLINE_LINMATH int compare_to(const FLOATNAME(LMatrix4) &other) const;
  int compare_to(const FLOATNAME(LMatrix4) &other, FLOATTYPE threshold) const;

  INLINE_LINMATH FLOATNAME(LVecBase4)
  xform(const FLOATNAME(LVecBase4) &v) const;

  INLINE_LINMATH FLOATNAME(LVecBase3)
  xform_point(const FLOATNAME(LVecBase3) &v) const;

  INLINE_LINMATH FLOATNAME(LVecBase3)
  xform_vec(const FLOATNAME(LVecBase3) &v) const;

  // this = other1 * other2
  INLINE_LINMATH void multiply(const FLOATNAME(LMatrix4) &other1, const FLOATNAME(LMatrix4) &other2);

        // this = scale_mat(scale_vector) * other_mat, efficiently
  INLINE_LINMATH void scale_multiply(const FLOATNAME(LVecBase3) &scale_vector,const FLOATNAME(LMatrix4) &other_mat);

  INLINE_LINMATH FLOATNAME(LMatrix4) operator * (const FLOATNAME(LMatrix4) &other) const;

  INLINE_LINMATH FLOATNAME(LMatrix4) operator * (FLOATTYPE scalar) const;
  INLINE_LINMATH FLOATNAME(LMatrix4) operator / (FLOATTYPE scalar) const;

  INLINE_LINMATH FLOATNAME(LMatrix4) &operator += (const FLOATNAME(LMatrix4) &other);
  INLINE_LINMATH FLOATNAME(LMatrix4) &operator -= (const FLOATNAME(LMatrix4) &other);

  INLINE_LINMATH FLOATNAME(LMatrix4) &operator *= (const FLOATNAME(LMatrix4) &other);

  INLINE_LINMATH FLOATNAME(LMatrix4) &operator *= (FLOATTYPE scalar);
  INLINE_LINMATH FLOATNAME(LMatrix4) &operator /= (FLOATTYPE scalar);

  INLINE_LINMATH void transpose_from(const FLOATNAME(LMatrix4) &other);
  INLINE_LINMATH void transpose_in_place();

  INLINE_LINMATH bool invert_from(const FLOATNAME(LMatrix4) &other);
  INLINE_LINMATH bool invert_affine_from(const FLOATNAME(LMatrix4) &other);
  INLINE_LINMATH bool invert_in_place();

  INLINE_LINMATH static const FLOATNAME(LMatrix4) &ident_mat();
  INLINE_LINMATH static FLOATNAME(LMatrix4)
    translate_mat(const FLOATNAME(LVecBase3) &trans);
  INLINE_LINMATH static FLOATNAME(LMatrix4)
    translate_mat(FLOATTYPE tx, FLOATTYPE ty, FLOATTYPE tz);
  INLINE_LINMATH static FLOATNAME(LMatrix4)
    rotate_mat(FLOATTYPE angle,
               FLOATNAME(LVecBase3) axis,
               CoordinateSystem cs = CS_default);
  INLINE_LINMATH static FLOATNAME(LMatrix4)
    rotate_mat_normaxis(FLOATTYPE angle,
                        const FLOATNAME(LVecBase3) &axis,
                        CoordinateSystem cs = CS_default);
  INLINE_LINMATH static void
    rotate_mat_normaxis(FLOATTYPE angle,
                        const FLOATNAME(LVecBase3) &axis,
                        FLOATNAME(LMatrix4) &result_mat,
                        CoordinateSystem cs = CS_default);
  INLINE_LINMATH static FLOATNAME(LMatrix4)
    scale_mat(const FLOATNAME(LVecBase3) &scale);
  INLINE_LINMATH static FLOATNAME(LMatrix4)
    scale_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz);
  INLINE_LINMATH static FLOATNAME(LMatrix4)
    scale_mat(FLOATTYPE scale);

  static INLINE_LINMATH FLOATNAME(LMatrix4)
    shear_mat(const FLOATNAME(LVecBase3) &shear);
  static INLINE_LINMATH FLOATNAME(LMatrix4)
    shear_mat(FLOATTYPE shxy, FLOATTYPE shxz, FLOATTYPE shyz);

  INLINE_LINMATH static const FLOATNAME(LMatrix4) &y_to_z_up_mat();
  INLINE_LINMATH static const FLOATNAME(LMatrix4) &z_to_y_up_mat();

  static const FLOATNAME(LMatrix4) &convert_mat(CoordinateSystem from,
                                                CoordinateSystem to);

  bool almost_equal(const FLOATNAME(LMatrix4) &other,
                    FLOATTYPE threshold) const;
  INLINE_LINMATH bool almost_equal(const FLOATNAME(LMatrix4) &other) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

public:
  INLINE_LINMATH void generate_hash(ChecksumHashGenerator &hashgen) const;
  void generate_hash(ChecksumHashGenerator &hashgen, FLOATTYPE scale) const;

public:
  union {
    struct {
      FLOATTYPE  _00, _01, _02, _03;
      FLOATTYPE  _10, _11, _12, _13;
      FLOATTYPE  _20, _21, _22, _23;
      FLOATTYPE  _30, _31, _32, _33;
    } m;
    
    FLOATTYPE data[4 * 4];
  } _m;

private:
  INLINE_LINMATH FLOATTYPE mult_cel(const FLOATNAME(LMatrix4) &other, int x, int y) const;
  bool decompose_mat(int index[4]);
  bool back_sub_mat(int index[4], FLOATNAME(LMatrix4) &inv, int row) const;

  static const FLOATNAME(LMatrix4) _ident_mat;
  static const FLOATNAME(LMatrix4) _y_to_z_up_mat;
  static const FLOATNAME(LMatrix4) _z_to_y_up_mat;
  static const FLOATNAME(LMatrix4) _flip_y_mat;
  static const FLOATNAME(LMatrix4) _flip_z_mat;
  static const FLOATNAME(LMatrix4) _lz_to_ry_mat;
  static const FLOATNAME(LMatrix4) _ly_to_rz_mat;

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


INLINE_LINMATH ostream &operator << (ostream &out, const FLOATNAME(LMatrix4) &mat) {
  mat.output(out);
  return out;
}


INLINE_LINMATH FLOATNAME(LMatrix4) transpose(const FLOATNAME(LMatrix4) &a);
INLINE_LINMATH FLOATNAME(LMatrix4) invert(const FLOATNAME(LMatrix4) &a);

#include "lmatrix4_src.I"
