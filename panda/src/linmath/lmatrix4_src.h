/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lmatrix4_src.h
 * @author drose
 * @date 1999-01-15
 */

class FLOATNAME(UnalignedLMatrix4);

/**
 * This is a 4-by-4 transform matrix.
 */
class EXPCL_PANDA_LINMATH ALIGN_LINMATH FLOATNAME(LMatrix4) {
public:
  typedef FLOATTYPE numeric_type;
  typedef const FLOATTYPE *iterator;
  typedef const FLOATTYPE *const_iterator;

PUBLISHED:
  enum {
    num_components = 16,
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
    INLINE_LINMATH operator const FLOATNAME(LVecBase4) &() const;
  public:
    FLOATTYPE *_row;
    friend class FLOATNAME(LMatrix4);
  };
  class CRow {
  private:
    INLINE_LINMATH CRow(const FLOATTYPE *row);
  PUBLISHED:
    INLINE_LINMATH FLOATTYPE operator [](int i) const;
    INLINE_LINMATH static int size();
    INLINE_LINMATH operator const FLOATNAME(LVecBase4) &() const;
  public:
    const FLOATTYPE *_row;
    friend class FLOATNAME(LMatrix4);
  };

  INLINE_LINMATH FLOATNAME(LMatrix4)() = default;
  INLINE_LINMATH FLOATNAME(LMatrix4)(const FLOATNAME(LMatrix4) &other) = default;
  INLINE_LINMATH FLOATNAME(LMatrix4)(const FLOATNAME(UnalignedLMatrix4) &other);
  INLINE_LINMATH FLOATNAME(LMatrix4) &operator = (
      const FLOATNAME(LMatrix4) &other) = default;
  INLINE_LINMATH FLOATNAME(LMatrix4) &operator = (
      const FLOATNAME(UnalignedLMatrix4) &other);
  INLINE_LINMATH FLOATNAME(LMatrix4) &operator = (FLOATTYPE fill_value);

  INLINE_LINMATH FLOATNAME(LMatrix4)(FLOATTYPE, FLOATTYPE, FLOATTYPE, FLOATTYPE,
                                     FLOATTYPE, FLOATTYPE, FLOATTYPE, FLOATTYPE,
                                     FLOATTYPE, FLOATTYPE, FLOATTYPE, FLOATTYPE,
                                     FLOATTYPE, FLOATTYPE, FLOATTYPE, FLOATTYPE);
  INLINE_LINMATH FLOATNAME(LMatrix4)(const FLOATNAME(LVecBase4) &,
                                     const FLOATNAME(LVecBase4) &,
                                     const FLOATNAME(LVecBase4) &,
                                     const FLOATNAME(LVecBase4) &);
  ALLOC_DELETED_CHAIN(FLOATNAME(LMatrix4));

  EXTENSION(INLINE_LINMATH PyObject *__reduce__(PyObject *self) const);

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

  INLINE_LINMATH CRow operator [](int i) const;
  INLINE_LINMATH Row operator [](int i);
  INLINE_LINMATH static int size();

  INLINE_LINMATH void set_row(int row, const FLOATNAME(LVecBase4) &v);
  INLINE_LINMATH void set_col(int col, const FLOATNAME(LVecBase4) &v);

  INLINE_LINMATH void set_row(int row, const FLOATNAME(LVecBase3) &v);
  INLINE_LINMATH void set_col(int col, const FLOATNAME(LVecBase3) &v);

  INLINE_LINMATH FLOATNAME(LVecBase4) get_row(int row) const;
  INLINE_LINMATH FLOATNAME(LVecBase4) get_col(int col) const;
  INLINE_LINMATH FLOATNAME(LVecBase3) get_row3(int row) const;
  MAKE_SEQ(get_rows, size, get_row);
  MAKE_SEQ(get_cols, size, get_col);
  MAKE_SEQ(get_row3s, size, get_row3);
  MAKE_SEQ_PROPERTY(rows, size, get_row);
  MAKE_SEQ_PROPERTY(cols, size, get_col);

  // these versions inline better
  INLINE_LINMATH void get_row(FLOATNAME(LVecBase4) &result_vec, int row) const;
  INLINE_LINMATH void get_row3(FLOATNAME(LVecBase3) &result_vec, int row) const;

  INLINE_LINMATH FLOATNAME(LVecBase3) get_col3(int col) const;

  INLINE_LINMATH FLOATTYPE &operator () (int row, int col);
  INLINE_LINMATH FLOATTYPE operator () (int row, int col) const;

  INLINE_LINMATH bool is_nan() const;
  INLINE_LINMATH bool is_identity() const;

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
  INLINE_LINMATH size_t get_hash() const;
  INLINE_LINMATH size_t get_hash(FLOATTYPE threshold) const;
  INLINE_LINMATH size_t add_hash(size_t hash) const;
  INLINE_LINMATH size_t add_hash(size_t hash, FLOATTYPE threshold) const;

  INLINE_LINMATH FLOATNAME(LVecBase4)
  xform(const FLOATNAME(LVecBase4) &v) const;

  INLINE_LINMATH FLOATNAME(LVecBase3)
  xform_point(const FLOATNAME(LVecBase3) &v) const;

  INLINE_LINMATH FLOATNAME(LVecBase3)
  xform_point_general(const FLOATNAME(LVecBase3) &v) const;

  INLINE_LINMATH FLOATNAME(LVecBase3)
  xform_vec(const FLOATNAME(LVecBase3) &v) const;

  INLINE_LINMATH FLOATNAME(LVecBase3)
  xform_vec_general(const FLOATNAME(LVecBase3) &v) const;

  INLINE_LINMATH void
  xform_in_place(FLOATNAME(LVecBase4) &v) const;

  INLINE_LINMATH void
  xform_point_in_place(FLOATNAME(LVecBase3) &v) const;

  INLINE_LINMATH void
  xform_point_general_in_place(FLOATNAME(LVecBase3) &v) const;

  INLINE_LINMATH void
  xform_vec_in_place(FLOATNAME(LVecBase3) &v) const;

  INLINE_LINMATH void
  xform_vec_general_in_place(FLOATNAME(LVecBase3) &v) const;

  // this = other1 * other2
  INLINE_LINMATH void multiply(const FLOATNAME(LMatrix4) &other1, const FLOATNAME(LMatrix4) &other2);

  INLINE_LINMATH FLOATNAME(LMatrix4) operator * (const FLOATNAME(LMatrix4) &other) const;

  INLINE_LINMATH FLOATNAME(LMatrix4) operator * (FLOATTYPE scalar) const;
  INLINE_LINMATH FLOATNAME(LMatrix4) operator / (FLOATTYPE scalar) const;

  INLINE_LINMATH FLOATNAME(LMatrix4) &operator += (const FLOATNAME(LMatrix4) &other);
  INLINE_LINMATH FLOATNAME(LMatrix4) &operator -= (const FLOATNAME(LMatrix4) &other);

  INLINE_LINMATH FLOATNAME(LMatrix4) &operator *= (const FLOATNAME(LMatrix4) &other);

  INLINE_LINMATH FLOATNAME(LMatrix4) &operator *= (FLOATTYPE scalar);
  INLINE_LINMATH FLOATNAME(LMatrix4) &operator /= (FLOATTYPE scalar);

  INLINE_LINMATH void componentwise_mult(const FLOATNAME(LMatrix4) &other);

  INLINE_LINMATH void transpose_from(const FLOATNAME(LMatrix4) &other);
  INLINE_LINMATH void transpose_in_place();

  INLINE_LINMATH bool invert_from(const FLOATNAME(LMatrix4) &other);
  INLINE_LINMATH bool invert_affine_from(const FLOATNAME(LMatrix4) &other);
  INLINE_LINMATH bool invert_in_place();

  INLINE_LINMATH void accumulate(const FLOATNAME(LMatrix4) &other, FLOATTYPE weight);

  INLINE_LINMATH static const FLOATNAME(LMatrix4) &ident_mat();
  INLINE_LINMATH static const FLOATNAME(LMatrix4) &ones_mat();
  INLINE_LINMATH static const FLOATNAME(LMatrix4) &zeros_mat();

  INLINE_LINMATH void
    set_translate_mat(const FLOATNAME(LVecBase3) &trans);
  void
    set_rotate_mat(FLOATTYPE angle,
                   const FLOATNAME(LVecBase3) &axis,
                   CoordinateSystem cs = CS_default);
  void
    set_rotate_mat_normaxis(FLOATTYPE angle,
                            const FLOATNAME(LVecBase3) &axis,
                            CoordinateSystem cs = CS_default);
  INLINE_LINMATH void
    set_scale_mat(const FLOATNAME(LVecBase3) &scale);
  INLINE_LINMATH void
    set_shear_mat(const FLOATNAME(LVecBase3) &shear,
                  CoordinateSystem cs = CS_default);
  INLINE_LINMATH void
    set_scale_shear_mat(const FLOATNAME(LVecBase3) &scale,
                        const FLOATNAME(LVecBase3) &shear,
                        CoordinateSystem cs = CS_default);

  INLINE_LINMATH static FLOATNAME(LMatrix4)
    translate_mat(const FLOATNAME(LVecBase3) &trans);
  INLINE_LINMATH static FLOATNAME(LMatrix4)
    translate_mat(FLOATTYPE tx, FLOATTYPE ty, FLOATTYPE tz);
  INLINE_LINMATH static FLOATNAME(LMatrix4)
    rotate_mat(FLOATTYPE angle,
               const FLOATNAME(LVecBase3) &axis,
               CoordinateSystem cs = CS_default);
  INLINE_LINMATH static FLOATNAME(LMatrix4)
    rotate_mat_normaxis(FLOATTYPE angle,
                        const FLOATNAME(LVecBase3) &axis,
                        CoordinateSystem cs = CS_default);
  INLINE_LINMATH static FLOATNAME(LMatrix4)
    scale_mat(const FLOATNAME(LVecBase3) &scale);
  INLINE_LINMATH static FLOATNAME(LMatrix4)
    scale_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz);
  INLINE_LINMATH static FLOATNAME(LMatrix4)
    scale_mat(FLOATTYPE scale);

  static INLINE_LINMATH FLOATNAME(LMatrix4)
    shear_mat(const FLOATNAME(LVecBase3) &shear,
              CoordinateSystem cs = CS_default);
  static INLINE_LINMATH FLOATNAME(LMatrix4)
    shear_mat(FLOATTYPE shxy, FLOATTYPE shxz, FLOATTYPE shyz,
              CoordinateSystem cs = CS_default);

  static INLINE_LINMATH FLOATNAME(LMatrix4)
    scale_shear_mat(const FLOATNAME(LVecBase3) &scale,
                    const FLOATNAME(LVecBase3) &shear,
                    CoordinateSystem cs = CS_default);
  static INLINE_LINMATH FLOATNAME(LMatrix4)
    scale_shear_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz,
                    FLOATTYPE shxy, FLOATTYPE shxz, FLOATTYPE shyz,
                    CoordinateSystem cs = CS_default);

  INLINE_LINMATH static const FLOATNAME(LMatrix4) &y_to_z_up_mat();
  INLINE_LINMATH static const FLOATNAME(LMatrix4) &z_to_y_up_mat();

  static const FLOATNAME(LMatrix4) &convert_mat(CoordinateSystem from,
                                                CoordinateSystem to);

  bool almost_equal(const FLOATNAME(LMatrix4) &other,
                    FLOATTYPE threshold) const;
  INLINE_LINMATH bool almost_equal(const FLOATNAME(LMatrix4) &other) const;

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;
  EXTENSION(INLINE_LINMATH std::string __repr__() const);

  INLINE_LINMATH void generate_hash(ChecksumHashGenerator &hashgen) const;
  void generate_hash(ChecksumHashGenerator &hashgen, FLOATTYPE scale) const;

  void write_datagram_fixed(Datagram &destination) const;
  void read_datagram_fixed(DatagramIterator &scan);
  void write_datagram(Datagram &destination) const;
  void read_datagram(DatagramIterator &source);

public:
  // The underlying implementation is via the Eigen library, if available.

  // Unlike LMatrix3, we fully align LMatrix4 to 16-byte boundaries, to take
  // advantage of SSE2 optimizations when available.  Sometimes this alignment
  // requirement is inconvenient, so we also provide UnalignedLMatrix4, below.
  typedef LINMATH_MATRIX(FLOATTYPE, 4, 4) EMatrix4;
  EMatrix4 _m;

  INLINE_LINMATH FLOATNAME(LMatrix4)(const EMatrix4 &m) : _m(m) { }

private:
  bool decompose_mat(int index[4]);
  bool back_sub_mat(int index[4], FLOATNAME(LMatrix4) &inv, int row) const;

  static const FLOATNAME(LMatrix4) _ident_mat;
  static const FLOATNAME(LMatrix4) _ones_mat;
  static const FLOATNAME(LMatrix4) _zeros_mat;
  static const FLOATNAME(LMatrix4) _y_to_z_up_mat;
  static const FLOATNAME(LMatrix4) _z_to_y_up_mat;
  static const FLOATNAME(LMatrix4) _flip_y_mat;
  static const FLOATNAME(LMatrix4) _flip_z_mat;
  static const FLOATNAME(LMatrix4) _lz_to_ry_mat;
  static const FLOATNAME(LMatrix4) _ly_to_rz_mat;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

/**
 * This is an "unaligned" LMatrix4.  It has no functionality other than to
 * store numbers, and it will pack them in as tightly as possible, avoiding
 * any SSE2 alignment requirements shared by the primary LMatrix4 class.
 *
 * Use it only when you need to pack numbers tightly without respect to
 * alignment, and then copy it to a proper LMatrix4 to get actual use from it.
 */
class EXPCL_PANDA_LINMATH FLOATNAME(UnalignedLMatrix4) {
PUBLISHED:
  enum {
    num_components = 16
  };

  INLINE_LINMATH FLOATNAME(UnalignedLMatrix4)() = default;
  INLINE_LINMATH FLOATNAME(UnalignedLMatrix4)(const FLOATNAME(LMatrix4) &copy);
  INLINE_LINMATH FLOATNAME(UnalignedLMatrix4)(const FLOATNAME(UnalignedLMatrix4) &copy) = default;
  INLINE_LINMATH FLOATNAME(UnalignedLMatrix4) &operator = (const FLOATNAME(LMatrix4) &copy);
  INLINE_LINMATH FLOATNAME(UnalignedLMatrix4) &operator = (const FLOATNAME(UnalignedLMatrix4) &copy) = default;
  INLINE_LINMATH FLOATNAME(UnalignedLMatrix4)(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02, FLOATTYPE e03,
                                              FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12, FLOATTYPE e13,
                                              FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22, FLOATTYPE e23,
                                              FLOATTYPE e30, FLOATTYPE e31, FLOATTYPE e32, FLOATTYPE e33);

  INLINE_LINMATH void set(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02, FLOATTYPE e03,
                          FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12, FLOATTYPE e13,
                          FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22, FLOATTYPE e23,
                          FLOATTYPE e30, FLOATTYPE e31, FLOATTYPE e32, FLOATTYPE e33);

  INLINE_LINMATH FLOATTYPE &operator () (int row, int col);
  INLINE_LINMATH FLOATTYPE operator () (int row, int col) const;

  INLINE_LINMATH const FLOATTYPE *get_data() const;
  INLINE_LINMATH int get_num_components() const;

  INLINE_LINMATH bool operator == (const FLOATNAME(UnalignedLMatrix4) &other) const;
  INLINE_LINMATH bool operator != (const FLOATNAME(UnalignedLMatrix4) &other) const;

public:
  typedef UNALIGNED_LINMATH_MATRIX(FLOATTYPE, 4, 4) UMatrix4;
  UMatrix4 _m;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};


INLINE std::ostream &operator << (std::ostream &out, const FLOATNAME(LMatrix4) &mat) {
  mat.output(out);
  return out;
}

BEGIN_PUBLISH
INLINE_LINMATH FLOATNAME(LMatrix4) transpose(const FLOATNAME(LMatrix4) &a);
INLINE_LINMATH FLOATNAME(LMatrix4) invert(const FLOATNAME(LMatrix4) &a);
END_PUBLISH

#include "lmatrix4_src.I"
