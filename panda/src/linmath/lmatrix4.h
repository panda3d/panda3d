// Filename: lmatrix4.h
// Created by:  drose (29Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef LMATRIX4_H
#define LMATRIX4_H

#include <pandabase.h>

#include "coordinateSystem.h"
#include "lvecBase4.h"
#include "lvecBase3.h"
#include "lmatrix3.h"

#include <typeHandle.h>
#include <datagram.h>
#include <datagramIterator.h>

////////////////////////////////////////////////////////////////////
// 	 Class : LMatrix4
// Description : This is a 4-by-4 transform matrix.
////////////////////////////////////////////////////////////////////
template<class NumType>
class LMatrix4 {
public:
  typedef const NumType *iterator;
  typedef const NumType *const_iterator;

  INLINE LMatrix4();
  INLINE LMatrix4(const LMatrix4<NumType> &other);
  LMatrix4<NumType> &operator = (const LMatrix4<NumType> &other);
  INLINE LMatrix4<NumType> &operator = (NumType fill_value);
  INLINE LMatrix4(NumType e00, NumType e01, NumType e02, NumType e03,
		  NumType e10, NumType e11, NumType e12, NumType e13,
		  NumType e20, NumType e21, NumType e22, NumType e23,
		  NumType e30, NumType e31, NumType e32, NumType e33);

  // Construct a 4x4 matrix given a 3x3 rotation matrix and an optional
  // translation component.
  LMatrix4(const LMatrix3<NumType> &upper3);
  LMatrix4(const LMatrix3<NumType> &upper3,
	   const LVecBase3<NumType> &trans);

  void fill(NumType fill_value);
  INLINE void set(NumType e00, NumType e01, NumType e02, NumType e03,
		  NumType e10, NumType e11, NumType e12, NumType e13,
		  NumType e20, NumType e21, NumType e22, NumType e23,
		  NumType e30, NumType e31, NumType e32, NumType e33);
 
  // Get and set the upper 3x3 rotation matrix.
  INLINE void set_upper_3(const LMatrix3<NumType> &upper3);
  INLINE LMatrix3<NumType> get_upper_3() const;

  INLINE void set_row(int row, const LVecBase4<NumType> &v);
  INLINE void set_col(int col, const LVecBase4<NumType> &v);

  INLINE void set_row(int row, const LVecBase3<NumType> &v);
  INLINE void set_col(int col, const LVecBase3<NumType> &v);

  INLINE LVecBase4<NumType> get_row(int row) const;
  INLINE LVecBase4<NumType> get_col(int col) const;

  INLINE LVecBase3<NumType> get_row3(int row) const;
  INLINE LVecBase3<NumType> get_col3(int col) const;

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


  bool operator == (const LMatrix4<NumType> &other) const;
  INLINE bool operator != (const LMatrix4<NumType> &other) const;

  INLINE int compare_to(const LMatrix4<NumType> &other) const;
  int compare_to(const LMatrix4<NumType> &other, NumType threshold) const;

  INLINE LVecBase4<NumType>
  xform(const LVecBase4<NumType> &v) const;

  INLINE LVecBase3<NumType>
  xform_point(const LVecBase3<NumType> &v) const;
 
  INLINE LVecBase3<NumType>
  xform_vec(const LVecBase3<NumType> &v) const;

  LMatrix4<NumType> operator * (const LMatrix4<NumType> &other) const;
  LMatrix4<NumType> operator * (NumType scalar) const;
  LMatrix4<NumType> operator / (NumType scalar) const;

  LMatrix4<NumType> &operator += (const LMatrix4<NumType> &other);
  LMatrix4<NumType> &operator -= (const LMatrix4<NumType> &other);

  INLINE LMatrix4<NumType> &operator *= (const LMatrix4<NumType> &other);
  
  LMatrix4<NumType> &operator *= (NumType scalar);
  LMatrix4<NumType> &operator /= (NumType scalar);

  void transpose_from(const LMatrix4<NumType> &other);
  INLINE void transpose_in_place();

  bool invert_from(const LMatrix4<NumType> &other);
  bool invert_affine_from(const LMatrix4<NumType> &other);
  INLINE bool invert_in_place();

  static const LMatrix4<NumType> &ident_mat();
  static LMatrix4<NumType> translate_mat(const LVecBase3<NumType> &trans);
  static LMatrix4<NumType> translate_mat(NumType tx, NumType ty, NumType tz);
  static LMatrix4<NumType> rotate_mat(NumType angle,
				      LVecBase3<NumType> axis,
				      CoordinateSystem cs = CS_default);
  static LMatrix4<NumType> scale_mat(const LVecBase3<NumType> &scale);
  static LMatrix4<NumType> scale_mat(NumType sx, NumType sy, NumType sz);
  static LMatrix4<NumType> scale_mat(NumType scale);

  static const LMatrix4<NumType> &y_to_z_up_mat();
  static const LMatrix4<NumType> &z_to_y_up_mat();

  static LMatrix4<NumType> convert_mat(CoordinateSystem from,
				       CoordinateSystem to);


  bool almost_equal(const LMatrix4<NumType> &other, 
		    NumType threshold) const;
  INLINE bool almost_equal(const LMatrix4<NumType> &other) const;
  
  INLINE void output(ostream &out) const;
  INLINE void write(ostream &out, int indent_level = 0) const;

private:
  INLINE NumType mult_cel(const LMatrix4<NumType> &other, int x, int y) const;
  bool decompose_mat(int index[4]);
  bool back_sub_mat(int index[4], LMatrix4<NumType> &inv, int row) const;

  NumType _data[4 * 4];

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
INLINE ostream &operator << (ostream &out, const LMatrix4<NumType> &mat) {
  mat.output(out);
  return out;
}

template<class NumType>
INLINE LMatrix4<NumType> transpose(const LMatrix4<NumType> &a);

template<class NumType>
INLINE LMatrix4<NumType> invert(const LMatrix4<NumType> &a);

// Cast to a different numeric type
template<class NumType, class NumType2>
INLINE LMatrix4<NumType2> 
lcast_to(NumType2 *type, const LMatrix4<NumType> &source);


#include "lmatrix4.I"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LMatrix4<float>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LMatrix4<double>)

#endif
