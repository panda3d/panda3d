// Filename: lvecBase4.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LVECBASE4_H
#define LVECBASE4_H

#include <pandabase.h>

#include "cmath.h"

#include <typeHandle.h>

class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
// 	 Class : LVecBase4
// Description : This is the base class for all three-component
//               vectors and points.
////////////////////////////////////////////////////////////////////
template<class NumType>
class LVecBase4 {
public:
  typedef const NumType *iterator;
  typedef const NumType *const_iterator;

  INLINE LVecBase4();
  INLINE LVecBase4(const LVecBase4<NumType> &copy);
  INLINE LVecBase4<NumType> &operator = (const LVecBase4<NumType> &copy);
  INLINE LVecBase4<NumType> &operator = (NumType fill_value);
  INLINE LVecBase4(NumType fill_value);
  INLINE LVecBase4(NumType x, NumType y, NumType z, NumType w);

  INLINE static LVecBase4<NumType> zero();
  INLINE static LVecBase4<NumType> unit_x();
  INLINE static LVecBase4<NumType> unit_y();
  INLINE static LVecBase4<NumType> unit_z();
  INLINE static LVecBase4<NumType> unit_w();

  INLINE ~LVecBase4();

  INLINE NumType operator [](int i) const;
  INLINE NumType &operator [](int i);

  INLINE bool is_nan() const;

  INLINE NumType get_cell(int i) const;
  INLINE NumType get_x() const;
  INLINE NumType get_y() const;
  INLINE NumType get_z() const;
  INLINE NumType get_w() const;
  INLINE void set_cell(int i, NumType value);
  INLINE void set_x(NumType value);
  INLINE void set_y(NumType value);
  INLINE void set_z(NumType value);
  INLINE void set_w(NumType value);

  INLINE const NumType *get_data() const;
  INLINE int get_num_components() const;

  INLINE iterator begin();
  INLINE iterator end();

  INLINE const_iterator begin() const;
  INLINE const_iterator end() const;

  INLINE void fill(NumType fill_value);
  INLINE void set(NumType x, NumType y, NumType z, NumType w);

  INLINE NumType dot(const LVecBase4<NumType> &other) const;

  INLINE bool operator < (const LVecBase4<NumType> &other) const;
  INLINE bool operator == (const LVecBase4<NumType> &other) const;
  INLINE bool operator != (const LVecBase4<NumType> &other) const;

  INLINE int compare_to(const LVecBase4<NumType> &other) const;
  INLINE int compare_to(const LVecBase4<NumType> &other,
		        NumType threshold) const;

  INLINE LVecBase4<NumType> 
  operator - () const;

  INLINE LVecBase4<NumType>
  operator + (const LVecBase4<NumType> &other) const;
  INLINE LVecBase4<NumType>
  operator - (const LVecBase4<NumType> &other) const;

  INLINE LVecBase4<NumType> operator * (NumType scalar) const;
  INLINE LVecBase4<NumType> operator / (NumType scalar) const;

  INLINE void operator += (const LVecBase4<NumType> &other);
  INLINE void operator -= (const LVecBase4<NumType> &other);

  INLINE void operator *= (NumType scalar);
  INLINE void operator /= (NumType scalar);

  INLINE bool almost_equal(const LVecBase4<NumType> &other, 
			   NumType threshold) const;
  INLINE bool almost_equal(const LVecBase4<NumType> &other) const;

  INLINE void output(ostream &out) const;

private:
  NumType _data[4];

public:
  INLINE void write_datagram(Datagram &destination) const;
  INLINE void read_datagram(DatagramIterator &source);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
 
private:
  static TypeHandle _type_handle;
};

template<class NumType>
INLINE ostream &operator << (ostream &out, const LVecBase4<NumType> &vec) {
  vec.output(out);
  return out;
}


// Cast to a different numeric type
template<class NumType, class NumType2>
INLINE LVecBase4<NumType2> 
lcast_to(NumType2 *type, const LVecBase4<NumType> &source);

#include "lvecBase4.I"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LVecBase4<float>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LVecBase4<double>)

#endif
