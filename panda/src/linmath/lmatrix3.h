// Filename: lmatrix3.h
// Created by:  drose (29Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef LMATRIX3_H
#define LMATRIX3_H

#include <pandabase.h>

#include "coordinateSystem.h"
#include "lvecBase3.h"
#include "lvecBase2.h"

#include <typeHandle.h>
#include <datagram.h>
#include <datagramIterator.h>

////////////////////////////////////////////////////////////////////
// 	 Class : LMatrix3
// Description : This is a 3-by-3 transform matrix.  It typically will
//               represent either a rotation-and-scale (no
//               translation) matrix in 3-d, or a full affine matrix
//               (rotation, scale, translation) in 2-d, e.g. for a
//               texture matrix.
////////////////////////////////////////////////////////////////////
template<class NumType>
class LMatrix3 {
public:
  typedef const NumType *iterator;
  typedef const NumType *const_iterator;

  INLINE LMatrix3();
  INLINE LMatrix3(const LMatrix3<NumType> &other);
  LMatrix3<NumType> &operator = (const LMatrix3<NumType> &other);
  INLINE LMatrix3<NumType> &operator = (NumType fill_value);
  INLINE LMatrix3(NumType e00, NumType e01, NumType e02,
		  NumType e10, NumType e11, NumType e12,
		  NumType e20, NumType e21, NumType e22);

  void fill(NumType fill_value);
  INLINE void set(NumType e00, NumType e01, NumType e02,
		  NumType e10, NumType e11, NumType e12,
		  NumType e20, NumType e21, NumType e22);

  INLINE void set_row(int row, const LVecBase3<NumType> &v);
  INLINE void set_col(int col, const LVecBase3<NumType> &v);

  INLINE void set_row(int row, const LVecBase2<NumType> &v);
  INLINE void set_col(int col, const LVecBase2<NumType> &v);

  INLINE LVecBase3<NumType> get_row(int row) const;
  INLINE LVecBase3<NumType> get_col(int col) const;

  INLINE LVecBase2<NumType> get_row2(int row) const;
  INLINE LVecBase2<NumType> get_col2(int col) const;

  INLINE NumType &operator () (int row, int col);
  INLINE NumType operator () (int row, int col) const;

  INLINE bool is_nan() const;

  INLINE NumType get_cell(int row, int col) const;
  INLINE void set_cell(int row, int col, NumType value);

  INLINE const NumType *get_data() const;
  INLINE int get_num_components() const;

  INLINE iterator begin();
  INLINE iterator end();

  INLINE const_iterator begin() const;
  INLINE const_iterator end() const;


  bool operator == (const LMatrix3<NumType> &other) const;
  INLINE bool operator != (const LMatrix3<NumType> &other) const;

  INLINE int compare_to(const LMatrix3<NumType> &other) const;
  int compare_to(const LMatrix3<NumType> &other, NumType threshold) const;

  INLINE LVecBase3<NumType>
  xform(const LVecBase3<NumType> &v) const;

  INLINE LVecBase2<NumType>
  xform_point(const LVecBase2<NumType> &v) const;
 
  INLINE LVecBase2<NumType>
  xform_vec(const LVecBase2<NumType> &v) const;

  LMatrix3<NumType> operator * (const LMatrix3<NumType> &other) const;
  LMatrix3<NumType> operator * (NumType scalar) const;
  LMatrix3<NumType> operator / (NumType scalar) const;

  LMatrix3<NumType> &operator += (const LMatrix3<NumType> &other);
  LMatrix3<NumType> &operator -= (const LMatrix3<NumType> &other);

  INLINE LMatrix3<NumType> &operator *= (const LMatrix3<NumType> &other);
  
  LMatrix3<NumType> &operator *= (NumType scalar);
  LMatrix3<NumType> &operator /= (NumType scalar);

  INLINE NumType determinant() const;

  void transpose_from(const LMatrix3<NumType> &other);
  INLINE void transpose_in_place();

  bool invert_from(const LMatrix3<NumType> &other);
  INLINE bool invert_in_place();

  static const LMatrix3<NumType> &ident_mat();

  // A 3x3 matrix is likely to be used for one of two purposes.  In
  // 2-d coordinate space (e.g. texture or surface coordinates), it
  // can contain a full affine transform, with scale, rotate,
  // translate.  In 3-d coordinate space, it can contain only scale
  // and/or rotate; e.g., the upper 3x3 rectangle of a full 4x4
  // matrix.

  // The following named constructors return 3x3 matrices suitable for
  // affine transforms in 2-d coordinate space.

  static LMatrix3<NumType> translate_mat(const LVecBase2<NumType> &trans);
  static LMatrix3<NumType> translate_mat(NumType tx, NumType ty);
  static LMatrix3<NumType> rotate_mat(NumType angle);
  static LMatrix3<NumType> scale_mat(const LVecBase2<NumType> &scale);
  static LMatrix3<NumType> scale_mat(NumType sx, NumType sy);

  // The following named constructors return 3x3 matrices suitable for
  // scale/rotate transforms in 3-d coordinate space.
  static LMatrix3<NumType> rotate_mat(NumType angle,
				      LVecBase3<NumType> axis,
				      CoordinateSystem cs = CS_default);
  static LMatrix3<NumType> scale_mat(const LVecBase3<NumType> &scale);
  static LMatrix3<NumType> scale_mat(NumType sx, NumType sy, NumType sz);

  // We don't have a scale_mat() that takes a single uniform scale
  // parameter, because it would be ambiguous whether we mean a 2-d or
  // a 3-d scale.


  bool almost_equal(const LMatrix3<NumType> &other, 
		    NumType threshold) const;

  INLINE bool almost_equal(const LMatrix3<NumType> &other) const;
  
  INLINE void output(ostream &out) const;
  INLINE void write(ostream &out, int indent_level = 0) const;

private:
  INLINE NumType mult_cel(const LMatrix3<NumType> &other, int x, int y) const;
  INLINE NumType det2(NumType e00, NumType e01, NumType e10, NumType e11) const;

  NumType _data[3 * 3];

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

template<class NumType>
INLINE ostream &operator << (ostream &out, const LMatrix3<NumType> &mat) {
  mat.output(out);
  return out;
}

template<class NumType>
INLINE LMatrix3<NumType> transpose(const LMatrix3<NumType> &a);

template<class NumType>
INLINE LMatrix3<NumType> invert(const LMatrix3<NumType> &a);

// Cast to a different numeric type
template<class NumType, class NumType2>
INLINE LMatrix3<NumType2> 
lcast_to(NumType2 *type, const LMatrix3<NumType> &source);

#include "lmatrix3.I"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LMatrix3<float>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LMatrix3<double>)

#endif
