/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lmatrix3_src.h
 * @author drose
 * @date 1999-01-29
 */

class FLOATNAME(LMatrix4);

/**
 * This is a 3-by-3 transform matrix.  It typically will represent either a
 * rotation-and-scale (no translation) matrix in 3-d, or a full affine matrix
 * (rotation, scale, translation) in 2-d, e.g.  for a texture matrix.
 */
class EXPCL_PANDA_LINMATH FLOATNAME(LMatrix3) {
public:
  typedef FLOATTYPE numeric_type;
  typedef const FLOATTYPE *iterator;
  typedef const FLOATTYPE *const_iterator;

PUBLISHED:
  enum {
    num_components = 9,
    is_int = 0
  };

  // These helper classes are used to support two-level operator [].
  class Row {
  private:
    INLINE_LINMATH Row(FLOATTYPE *row);
  PUBLISHED:
    INLINE_LINMATH FLOATTYPE operator [](int i) const;
    INLINE_LINMATH FLOATTYPE &operator [](int i);
    INLINE_LINMATH static int size();
    INLINE_LINMATH operator const FLOATNAME(LVecBase3) &() const;
  public:
    FLOATTYPE *_row;
    friend class FLOATNAME(LMatrix3);
  };
  class CRow {
  private:
    INLINE_LINMATH CRow(const FLOATTYPE *row);
  PUBLISHED:
    INLINE_LINMATH FLOATTYPE operator [](int i) const;
    INLINE_LINMATH static int size();
    INLINE_LINMATH operator const FLOATNAME(LVecBase3) &() const;
  public:
    const FLOATTYPE *_row;
    friend class FLOATNAME(LMatrix3);
  };

  INLINE_LINMATH FLOATNAME(LMatrix3)() = default;
  INLINE_LINMATH FLOATNAME(LMatrix3)(const FLOATNAME(LMatrix3) &other) = default;
  INLINE_LINMATH FLOATNAME(LMatrix3) &operator = (
      const FLOATNAME(LMatrix3) &other) = default;
  INLINE_LINMATH FLOATNAME(LMatrix3) &operator = (FLOATTYPE fill_value);
  INLINE_LINMATH FLOATNAME(LMatrix3)(FLOATTYPE, FLOATTYPE, FLOATTYPE,
                                     FLOATTYPE, FLOATTYPE, FLOATTYPE,
                                     FLOATTYPE, FLOATTYPE, FLOATTYPE);
  INLINE_LINMATH FLOATNAME(LMatrix3)(const FLOATNAME(LVecBase3) &,
                                     const FLOATNAME(LVecBase3) &,
                                     const FLOATNAME(LVecBase3) &);
  ALLOC_DELETED_CHAIN(FLOATNAME(LMatrix3));

  EXTENSION(INLINE_LINMATH PyObject *__reduce__(PyObject *self) const);

  void fill(FLOATTYPE fill_value);
  INLINE_LINMATH void set(
    FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02,
    FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12,
    FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22);

  INLINE_LINMATH CRow operator [](int i) const;
  INLINE_LINMATH Row operator [](int i);
  INLINE_LINMATH static int size();

  INLINE_LINMATH void set_row(int row, const FLOATNAME(LVecBase3) &v);
  INLINE_LINMATH void set_col(int col, const FLOATNAME(LVecBase3) &v);

  INLINE_LINMATH void set_row(int row, const FLOATNAME(LVecBase2) &v);
  INLINE_LINMATH void set_col(int col, const FLOATNAME(LVecBase2) &v);

  INLINE_LINMATH FLOATNAME(LVecBase3) get_row(int row) const;
  INLINE_LINMATH FLOATNAME(LVecBase3) get_col(int col) const;
  MAKE_SEQ(get_rows, size, get_row);
  MAKE_SEQ(get_cols, size, get_col);
  MAKE_SEQ_PROPERTY(rows, size, get_row);
  MAKE_SEQ_PROPERTY(cols, size, get_col);

  INLINE_LINMATH FLOATNAME(LVecBase2) get_row2(int row) const;
  INLINE_LINMATH FLOATNAME(LVecBase2) get_col2(int col) const;
  MAKE_SEQ(get_col2s, size, get_col2);
  MAKE_SEQ(get_row2s, size, get_row2);

  // these versions inline better
  INLINE_LINMATH void get_row(
      FLOATNAME(LVecBase3) &result_vec, int row) const;

  INLINE_LINMATH FLOATTYPE &operator () (int row, int col);
  INLINE_LINMATH FLOATTYPE operator () (int row, int col) const;

  INLINE_LINMATH bool is_nan() const;
  INLINE_LINMATH bool is_identity() const;

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
  INLINE_LINMATH size_t get_hash() const;
  INLINE_LINMATH size_t get_hash(FLOATTYPE threshold) const;
  INLINE_LINMATH size_t add_hash(size_t hash) const;
  INLINE_LINMATH size_t add_hash(size_t hash, FLOATTYPE threshold) const;

  INLINE_LINMATH FLOATNAME(LVecBase3)
  xform(const FLOATNAME(LVecBase3) &v) const;

  INLINE_LINMATH FLOATNAME(LVecBase2)
  xform_point(const FLOATNAME(LVecBase2) &v) const;

  INLINE_LINMATH FLOATNAME(LVecBase2)
  xform_vec(const FLOATNAME(LVecBase2) &v) const;

  INLINE_LINMATH FLOATNAME(LVecBase3)
  xform_vec(const FLOATNAME(LVecBase3) &v) const;

  INLINE_LINMATH FLOATNAME(LVecBase3)
  xform_vec_general(const FLOATNAME(LVecBase3) &v) const;

  INLINE_LINMATH void
  xform_in_place(FLOATNAME(LVecBase3) &v) const;

  INLINE_LINMATH void
  xform_point_in_place(FLOATNAME(LVecBase2) &v) const;

  INLINE_LINMATH void
  xform_vec_in_place(FLOATNAME(LVecBase2) &v) const;

  INLINE_LINMATH void
  xform_vec_in_place(FLOATNAME(LVecBase3) &v) const;

  INLINE_LINMATH void
  xform_vec_general_in_place(FLOATNAME(LVecBase3) &v) const;

  // this = other1 * other2
  INLINE_LINMATH void multiply(
    const FLOATNAME(LMatrix3) &other1, const FLOATNAME(LMatrix3) &other2);

  INLINE_LINMATH FLOATNAME(LMatrix3) operator * (
    const FLOATNAME(LMatrix3) &other) const;
  INLINE_LINMATH FLOATNAME(LMatrix3) operator * (FLOATTYPE scalar) const;
  INLINE_LINMATH FLOATNAME(LMatrix3) operator / (FLOATTYPE scalar) const;

  INLINE_LINMATH FLOATNAME(LMatrix3) &operator += (
    const FLOATNAME(LMatrix3) &other);
  INLINE_LINMATH FLOATNAME(LMatrix3) &operator -= (
    const FLOATNAME(LMatrix3) &other);

  INLINE_LINMATH FLOATNAME(LMatrix3) &operator *= (
    const FLOATNAME(LMatrix3) &other);

  INLINE_LINMATH FLOATNAME(LMatrix3) &operator *= (FLOATTYPE scalar);
  INLINE_LINMATH FLOATNAME(LMatrix3) &operator /= (FLOATTYPE scalar);

  INLINE_LINMATH void componentwise_mult(const FLOATNAME(LMatrix3) &other);

  INLINE_LINMATH FLOATTYPE determinant() const;

  INLINE_LINMATH void transpose_from(const FLOATNAME(LMatrix3) &other);
  INLINE_LINMATH void transpose_in_place();

  INLINE_LINMATH bool invert_from(const FLOATNAME(LMatrix3) &other);
  INLINE_LINMATH bool invert_in_place();

  INLINE_LINMATH bool invert_transpose_from(const FLOATNAME(LMatrix3) &other);
  INLINE_LINMATH bool invert_transpose_from(const FLOATNAME(LMatrix4) &other);

  static INLINE_LINMATH const FLOATNAME(LMatrix3) &ident_mat();

  // A 3x3 matrix is likely to be used for one of two purposes.  In 2-d
  // coordinate space (e.g.  texture or surface coordinates), it can contain a
  // full affine transform, with scale, rotate, translate.  In 3-d coordinate
  // space, it can contain only scale andor rotate; e.g., the upper 3x3
  // rectangle of a full 4x4 matrix.

  // The following named constructors return 3x3 matrices suitable for affine
  // transforms in 2-d coordinate space.

  INLINE_LINMATH void
    set_translate_mat(const FLOATNAME(LVecBase2) &trans);
  INLINE_LINMATH void
    set_rotate_mat(FLOATTYPE angle);
  INLINE_LINMATH void
    set_scale_mat(const FLOATNAME(LVecBase2) &scale);

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
  // scalerotate transforms in 3-d coordinate space.
  void
    set_rotate_mat(FLOATTYPE angle,
                   const FLOATNAME(LVecBase3) &axis,
                   CoordinateSystem cs = CS_default);
  void
    set_rotate_mat_normaxis(FLOATTYPE angle,
                            const FLOATNAME(LVecBase3) &axis,
                            CoordinateSystem cs = CS_default);

  static INLINE_LINMATH FLOATNAME(LMatrix3)
    rotate_mat(FLOATTYPE angle,
               const FLOATNAME(LVecBase3) &axis,
               CoordinateSystem cs = CS_default);
  static INLINE_LINMATH FLOATNAME(LMatrix3)
    rotate_mat_normaxis(FLOATTYPE angle,
                        const FLOATNAME(LVecBase3) &axis,
                        CoordinateSystem cs = CS_default);

  INLINE_LINMATH void
    set_scale_mat(const FLOATNAME(LVecBase3) &scale);

  static INLINE_LINMATH FLOATNAME(LMatrix3)
    scale_mat(const FLOATNAME(LVecBase3) &scale);
  static INLINE_LINMATH FLOATNAME(LMatrix3)
    scale_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz);

  INLINE_LINMATH void
    set_shear_mat(const FLOATNAME(LVecBase3) &shear,
                  CoordinateSystem cs = CS_default);

  static INLINE_LINMATH FLOATNAME(LMatrix3)
    shear_mat(const FLOATNAME(LVecBase3) &shear,
              CoordinateSystem cs = CS_default);
  static INLINE_LINMATH FLOATNAME(LMatrix3)
    shear_mat(FLOATTYPE shxy, FLOATTYPE shxz, FLOATTYPE shyz,
              CoordinateSystem cs = CS_default);

  void
    set_scale_shear_mat(const FLOATNAME(LVecBase3) &scale,
                        const FLOATNAME(LVecBase3) &shear,
                        CoordinateSystem cs = CS_default);

  static INLINE_LINMATH FLOATNAME(LMatrix3)
    scale_shear_mat(const FLOATNAME(LVecBase3) &scale,
                    const FLOATNAME(LVecBase3) &shear,
                    CoordinateSystem cs = CS_default);
  static INLINE_LINMATH FLOATNAME(LMatrix3)
    scale_shear_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz,
                    FLOATTYPE shxy, FLOATTYPE shxz, FLOATTYPE shyz,
                    CoordinateSystem cs = CS_default);

  static const FLOATNAME(LMatrix3) &convert_mat(CoordinateSystem from,
                                                CoordinateSystem to);

  // We don't have a scale_mat() that takes a single uniform scale parameter,
  // because it would be ambiguous whether we mean a 2-d or a 3-d scale.

  bool almost_equal(const FLOATNAME(LMatrix3) &other,
                    FLOATTYPE threshold) const;

  INLINE_LINMATH bool almost_equal(const FLOATNAME(LMatrix3) &other) const;

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;
  EXTENSION(INLINE_LINMATH std::string __repr__() const);

  INLINE_LINMATH void generate_hash(ChecksumHashGenerator &hashgen) const;
  void generate_hash(
    ChecksumHashGenerator &hashgen, FLOATTYPE threshold) const;

  void write_datagram_fixed(Datagram &destination) const;
  void read_datagram_fixed(DatagramIterator &scan);
  void write_datagram(Datagram &destination) const;
  void read_datagram(DatagramIterator &source);

public:
  // The underlying implementation is via the Eigen library, if available.

  // We don't bother to align LMatrix3, since it won't benefit from SSE2
  // optimizations anyway (it's an add number of floats).
  typedef UNALIGNED_LINMATH_MATRIX(FLOATTYPE, 3, 3) EMatrix3;
  EMatrix3 _m;

  INLINE_LINMATH FLOATNAME(LMatrix3)(const EMatrix3 &m) : _m(m) { }

private:
  static const FLOATNAME(LMatrix3) _ident_mat;
  static const FLOATNAME(LMatrix3) _y_to_z_up_mat;
  static const FLOATNAME(LMatrix3) _z_to_y_up_mat;
  static const FLOATNAME(LMatrix3) _flip_y_mat;
  static const FLOATNAME(LMatrix3) _flip_z_mat;
  static const FLOATNAME(LMatrix3) _lz_to_ry_mat;
  static const FLOATNAME(LMatrix3) _ly_to_rz_mat;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const FLOATNAME(LMatrix3) &mat) {
  mat.output(out);
  return out;
}

BEGIN_PUBLISH
INLINE_LINMATH FLOATNAME(LMatrix3) transpose(const FLOATNAME(LMatrix3) &a);
INLINE_LINMATH FLOATNAME(LMatrix3) invert(const FLOATNAME(LMatrix3) &a);
END_PUBLISH

// We can safely include lmatrix4_src.h down here and avoid circular
// dependencies.
#include "lmatrix4_src.h"

#include "lmatrix3_src.I"
