// Filename: lmatrix3_src.h
// Created by:  drose (29Jan99)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// 	 Class : LMatrix3
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

  INLINE FLOATNAME(LMatrix3)();
  INLINE FLOATNAME(LMatrix3)(const FLOATNAME(LMatrix3) &other);
  INLINE FLOATNAME(LMatrix3) &operator = (const FLOATNAME(LMatrix3) &other);
  INLINE FLOATNAME(LMatrix3) &operator = (FLOATTYPE fill_value);
  INLINE FLOATNAME(LMatrix3)(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02,
			     FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12,
			     FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22);

  void fill(FLOATTYPE fill_value);
  INLINE void set(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02,
		  FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12,
		  FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22);

  INLINE void set_row(int row, const FLOATNAME(LVecBase3) &v);
  INLINE void set_col(int col, const FLOATNAME(LVecBase3) &v);

  INLINE void set_row(int row, const FLOATNAME(LVecBase2) &v);
  INLINE void set_col(int col, const FLOATNAME(LVecBase2) &v);

  INLINE FLOATNAME(LVecBase3) get_row(int row) const;
  INLINE FLOATNAME(LVecBase3) get_col(int col) const;

  INLINE FLOATNAME(LVecBase2) get_row2(int row) const;
  INLINE FLOATNAME(LVecBase2) get_col2(int col) const;

  INLINE FLOATTYPE &operator () (int row, int col);
  INLINE FLOATTYPE operator () (int row, int col) const;

  INLINE bool is_nan() const;

  INLINE FLOATTYPE get_cell(int row, int col) const;
  INLINE void set_cell(int row, int col, FLOATTYPE value);

  INLINE const FLOATTYPE *get_data() const;
  INLINE int get_num_components() const;

public:
  INLINE iterator begin();
  INLINE iterator end();

  INLINE const_iterator begin() const;
  INLINE const_iterator end() const;

PUBLISHED:
  bool operator == (const FLOATNAME(LMatrix3) &other) const;
  INLINE bool operator != (const FLOATNAME(LMatrix3) &other) const;

  INLINE int compare_to(const FLOATNAME(LMatrix3) &other) const;
  int compare_to(const FLOATNAME(LMatrix3) &other, FLOATTYPE threshold) const;

  INLINE FLOATNAME(LVecBase3)
  xform(const FLOATNAME(LVecBase3) &v) const;

  INLINE FLOATNAME(LVecBase2)
  xform_point(const FLOATNAME(LVecBase2) &v) const;
 
  INLINE FLOATNAME(LVecBase2)
  xform_vec(const FLOATNAME(LVecBase2) &v) const;

  INLINE FLOATNAME(LMatrix3) operator * (const FLOATNAME(LMatrix3) &other) const;
  INLINE FLOATNAME(LMatrix3) operator * (FLOATTYPE scalar) const;
  INLINE FLOATNAME(LMatrix3) operator / (FLOATTYPE scalar) const;

  INLINE FLOATNAME(LMatrix3) &operator += (const FLOATNAME(LMatrix3) &other);
  INLINE FLOATNAME(LMatrix3) &operator -= (const FLOATNAME(LMatrix3) &other);

  INLINE FLOATNAME(LMatrix3) &operator *= (const FLOATNAME(LMatrix3) &other);
  
  INLINE FLOATNAME(LMatrix3) &operator *= (FLOATTYPE scalar);
  INLINE FLOATNAME(LMatrix3) &operator /= (FLOATTYPE scalar);

  INLINE FLOATTYPE determinant() const;

  INLINE void transpose_from(const FLOATNAME(LMatrix3) &other);
  INLINE void transpose_in_place();

  INLINE bool invert_from(const FLOATNAME(LMatrix3) &other);
  INLINE bool invert_in_place();

  static INLINE const FLOATNAME(LMatrix3) &ident_mat();

  // A 3x3 matrix is likely to be used for one of two purposes.  In
  // 2-d coordinate space (e.g. texture or surface coordinates), it
  // can contain a full affine transform, with scale, rotate,
  // translate.  In 3-d coordinate space, it can contain only scale
  // and/or rotate; e.g., the upper 3x3 rectangle of a full 4x4
  // matrix.

  // The following named constructors return 3x3 matrices suitable for
  // affine transforms in 2-d coordinate space.

  static INLINE FLOATNAME(LMatrix3) translate_mat(const FLOATNAME(LVecBase2) &trans);
  static INLINE FLOATNAME(LMatrix3) translate_mat(FLOATTYPE tx, FLOATTYPE ty);
  static INLINE FLOATNAME(LMatrix3) rotate_mat(FLOATTYPE angle);
  static INLINE FLOATNAME(LMatrix3) scale_mat(const FLOATNAME(LVecBase2) &scale);
  static INLINE FLOATNAME(LMatrix3) scale_mat(FLOATTYPE sx, FLOATTYPE sy);

  // The following named constructors return 3x3 matrices suitable for
  // scale/rotate transforms in 3-d coordinate space.
  static INLINE FLOATNAME(LMatrix3) rotate_mat(FLOATTYPE angle,
				      FLOATNAME(LVecBase3) axis,
				      CoordinateSystem cs = CS_default);
  static INLINE FLOATNAME(LMatrix3) scale_mat(const FLOATNAME(LVecBase3) &scale);
  static INLINE FLOATNAME(LMatrix3) scale_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz);

  // We don't have a scale_mat() that takes a single uniform scale
  // parameter, because it would be ambiguous whether we mean a 2-d or
  // a 3-d scale.

  bool almost_equal(const FLOATNAME(LMatrix3) &other, 
		    FLOATTYPE threshold) const;

  INLINE bool almost_equal(const FLOATNAME(LMatrix3) &other) const;
  
  INLINE void output(ostream &out) const;
  INLINE void write(ostream &out, int indent_level = 0) const;

private:
  INLINE FLOATTYPE mult_cel(const FLOATNAME(LMatrix3) &other, int x, int y) const;
  INLINE FLOATTYPE det2(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e10, FLOATTYPE e11) const;

  FLOATTYPE _data[3 * 3];
  static FLOATNAME(LMatrix3) _ident_mat;

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


INLINE ostream &operator << (ostream &out, const FLOATNAME(LMatrix3) &mat) {
  mat.output(out);
  return out;
}

INLINE FLOATNAME(LMatrix3) transpose(const FLOATNAME(LMatrix3) &a);
INLINE FLOATNAME(LMatrix3) invert(const FLOATNAME(LMatrix3) &a);

#include "lmatrix3_src.I"
