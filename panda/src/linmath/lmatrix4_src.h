// Filename: lmatrix4_src.h
// Created by:  drose (15Jan99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
// 	 Class : LMatrix4
// Description : This is a 4-by-4 transform matrix.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LMatrix4) {
PUBLISHED:
  typedef const FLOATTYPE *iterator;
  typedef const FLOATTYPE *const_iterator;

  INLINE FLOATNAME(LMatrix4)();
  INLINE FLOATNAME(LMatrix4)(const FLOATNAME(LMatrix4) &other);
  INLINE FLOATNAME(LMatrix4) &operator = (const FLOATNAME(LMatrix4) &other);
  INLINE FLOATNAME(LMatrix4) &operator = (FLOATTYPE fill_value);

  INLINE FLOATNAME(LMatrix4)(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02, FLOATTYPE e03,
			     FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12, FLOATTYPE e13,
			     FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22, FLOATTYPE e23,
			     FLOATTYPE e30, FLOATTYPE e31, FLOATTYPE e32, FLOATTYPE e33);

  // Construct a 4x4 matrix given a 3x3 rotation matrix and an optional
  // translation component.
  INLINE FLOATNAME(LMatrix4)(const FLOATNAME(LMatrix3) &upper3);
  INLINE FLOATNAME(LMatrix4)(const FLOATNAME(LMatrix3) &upper3,const FLOATNAME(LVecBase3) &trans);

  INLINE void fill(FLOATTYPE fill_value);
  INLINE void set(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02, FLOATTYPE e03,
		  FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12, FLOATTYPE e13,
		  FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22, FLOATTYPE e23,
		  FLOATTYPE e30, FLOATTYPE e31, FLOATTYPE e32, FLOATTYPE e33);
 
  // Get and set the upper 3x3 rotation matrix.
  INLINE void set_upper_3(const FLOATNAME(LMatrix3) &upper3);
  INLINE FLOATNAME(LMatrix3) get_upper_3() const;

  INLINE void set_row(int row, const FLOATNAME(LVecBase4) &v);
  INLINE void set_col(int col, const FLOATNAME(LVecBase4) &v);

  INLINE void set_row(int row, const FLOATNAME(LVecBase3) &v);
  INLINE void set_col(int col, const FLOATNAME(LVecBase3) &v);

  INLINE FLOATNAME(LVecBase4) get_row(int row) const;
  INLINE FLOATNAME(LVecBase4) get_col(int col) const;

  INLINE FLOATNAME(LVecBase3) get_row3(int row) const;
  INLINE FLOATNAME(LVecBase3) get_col3(int col) const;

  INLINE FLOATTYPE &operator () (int row, int col);
  INLINE FLOATTYPE operator () (int row, int col) const;

  INLINE bool is_nan() const;

  INLINE FLOATTYPE get_cell(int row, int col) const;
  INLINE void set_cell(int row, int col, FLOATTYPE value);

  INLINE const FLOATTYPE *get_data() const;
  INLINE int get_num_components() const;

  INLINE iterator begin();
  INLINE iterator end();

  INLINE const_iterator begin() const;
  INLINE const_iterator end() const;

  INLINE bool operator < (const FLOATNAME(LMatrix4) &other) const;
  INLINE bool operator == (const FLOATNAME(LMatrix4) &other) const;
  INLINE bool operator != (const FLOATNAME(LMatrix4) &other) const;

  INLINE int compare_to(const FLOATNAME(LMatrix4) &other) const;
  int compare_to(const FLOATNAME(LMatrix4) &other, FLOATTYPE threshold) const;

  INLINE FLOATNAME(LVecBase4)
  xform(const FLOATNAME(LVecBase4) &v) const;

  INLINE FLOATNAME(LVecBase3)
  xform_point(const FLOATNAME(LVecBase3) &v) const;
 
  INLINE FLOATNAME(LVecBase3)
  xform_vec(const FLOATNAME(LVecBase3) &v) const;

  INLINE FLOATNAME(LMatrix4) operator * (const FLOATNAME(LMatrix4) &other) const;
  INLINE FLOATNAME(LMatrix4) operator * (FLOATTYPE scalar) const;
  INLINE FLOATNAME(LMatrix4) operator / (FLOATTYPE scalar) const;

  INLINE FLOATNAME(LMatrix4) &operator += (const FLOATNAME(LMatrix4) &other);
  INLINE FLOATNAME(LMatrix4) &operator -= (const FLOATNAME(LMatrix4) &other);

  INLINE FLOATNAME(LMatrix4) &operator *= (const FLOATNAME(LMatrix4) &other);

  INLINE FLOATNAME(LMatrix4) &operator *= (FLOATTYPE scalar);
  INLINE FLOATNAME(LMatrix4) &operator /= (FLOATTYPE scalar);

  INLINE void transpose_from(const FLOATNAME(LMatrix4) &other);
  INLINE void transpose_in_place();

  INLINE bool invert_from(const FLOATNAME(LMatrix4) &other);
  INLINE bool invert_affine_from(const FLOATNAME(LMatrix4) &other);
  INLINE bool invert_in_place();

  INLINE static const FLOATNAME(LMatrix4) &ident_mat();
  INLINE static FLOATNAME(LMatrix4) translate_mat(const FLOATNAME(LVecBase3) &trans);
  INLINE static FLOATNAME(LMatrix4) translate_mat(FLOATTYPE tx, FLOATTYPE ty, FLOATTYPE tz);
  INLINE static FLOATNAME(LMatrix4) rotate_mat(FLOATTYPE angle,
				      FLOATNAME(LVecBase3) axis,
				      CoordinateSystem cs = CS_default);
  INLINE static FLOATNAME(LMatrix4) scale_mat(const FLOATNAME(LVecBase3) &scale);
  INLINE static FLOATNAME(LMatrix4) scale_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz);
  INLINE static FLOATNAME(LMatrix4) scale_mat(FLOATTYPE scale);

  INLINE static const FLOATNAME(LMatrix4) &y_to_z_up_mat();
  INLINE static const FLOATNAME(LMatrix4) &z_to_y_up_mat();

  static FLOATNAME(LMatrix4) convert_mat(CoordinateSystem from,
				       CoordinateSystem to);

  bool almost_equal(const FLOATNAME(LMatrix4) &other, 
		    FLOATTYPE threshold) const;
  INLINE bool almost_equal(const FLOATNAME(LMatrix4) &other) const;
  
  INLINE void output(ostream &out) const;
  INLINE void write(ostream &out, int indent_level = 0) const;

private:
  INLINE FLOATTYPE mult_cel(const FLOATNAME(LMatrix4) &other, int x, int y) const;
  bool decompose_mat(int index[4]);
  bool back_sub_mat(int index[4], FLOATNAME(LMatrix4) &inv, int row) const;

  FLOATTYPE _data[4 * 4];
  static const FLOATNAME(LMatrix4) _ident_mat;
  static const FLOATNAME(LMatrix4) _y_to_z_up_mat;
  static const FLOATNAME(LMatrix4) _z_to_y_up_mat;

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

INLINE ostream &operator << (ostream &out, const FLOATNAME(LMatrix4) &mat) {
  mat.output(out);
  return out;
}

INLINE FLOATNAME(LMatrix4) transpose(const FLOATNAME(LMatrix4) &a);
INLINE FLOATNAME(LMatrix4) invert(const FLOATNAME(LMatrix4) &a);

#include "lmatrix4_src.I"
