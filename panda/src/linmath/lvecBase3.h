// Filename: lvecBase3.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LVECBASE3_H
#define LVECBASE3_H

#include <pandabase.h>

#include "cmath.h"

#include <typeHandle.h>

class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
// 	 Class : LVecBase3
// Description : This is the base class for all three-component
//               vectors and points.
////////////////////////////////////////////////////////////////////
template<class NumType>
class LVecBase3 {
public:
  typedef const NumType *iterator;
  typedef const NumType *const_iterator;

  INLINE LVecBase3();
  INLINE LVecBase3(const LVecBase3<NumType> &copy);
  INLINE LVecBase3<NumType> &operator = (const LVecBase3<NumType> &copy);
  INLINE LVecBase3<NumType> &operator = (NumType fill_value);
  INLINE LVecBase3(NumType fill_value);
  INLINE LVecBase3(NumType x, NumType y, NumType z);

  INLINE static LVecBase3<NumType> zero();
  INLINE static LVecBase3<NumType> unit_x();
  INLINE static LVecBase3<NumType> unit_y();
  INLINE static LVecBase3<NumType> unit_z();

  INLINE ~LVecBase3();

  INLINE NumType operator [](int i) const;
  INLINE NumType &operator [](int i);

  INLINE bool is_nan() const;

  INLINE NumType get_cell(int i) const;
  INLINE NumType get_x() const;
  INLINE NumType get_y() const;
  INLINE NumType get_z() const;
  INLINE void set_cell(int i, NumType value);
  INLINE void set_x(NumType value);
  INLINE void set_y(NumType value);
  INLINE void set_z(NumType value);

  INLINE const NumType *get_data() const;
  INLINE int get_num_components() const;

  INLINE iterator begin();
  INLINE iterator end();

  INLINE const_iterator begin() const;
  INLINE const_iterator end() const;

  INLINE void fill(NumType fill_value);
  INLINE void set(NumType x, NumType y, NumType z);

  INLINE NumType dot(const LVecBase3<NumType> &other) const;
  INLINE LVecBase3<NumType> cross(const LVecBase3<NumType> &other) const;

  INLINE bool operator < (const LVecBase3<NumType> &other) const;
  INLINE bool operator == (const LVecBase3<NumType> &other) const;
  INLINE bool operator != (const LVecBase3<NumType> &other) const;

  INLINE int compare_to(const LVecBase3<NumType> &other) const;
  INLINE int compare_to(const LVecBase3<NumType> &other,
		        NumType threshold) const;

  INLINE LVecBase3<NumType> 
  operator - () const;

  INLINE LVecBase3<NumType>
  operator + (const LVecBase3<NumType> &other) const;
  INLINE LVecBase3<NumType>
  operator - (const LVecBase3<NumType> &other) const;

  INLINE LVecBase3<NumType> operator * (NumType scalar) const;
  INLINE LVecBase3<NumType> operator / (NumType scalar) const;

  INLINE void operator += (const LVecBase3<NumType> &other);
  INLINE void operator -= (const LVecBase3<NumType> &other);

  INLINE void operator *= (NumType scalar);
  INLINE void operator /= (NumType scalar);

  INLINE void cross_into(const LVecBase3<NumType> &other);

  INLINE bool almost_equal(const LVecBase3<NumType> &other, 
			   NumType threshold) const;
  INLINE bool almost_equal(const LVecBase3<NumType> &other) const;

  INLINE void output(ostream &out) const;

private:
  NumType _data[3];

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
INLINE ostream &operator << (ostream &out, const LVecBase3<NumType> &vec) {
  vec.output(out);
  return out;
}


// Cast to a different numeric type
template<class NumType, class NumType2>
INLINE LVecBase3<NumType2> 
lcast_to(NumType2 *type, const LVecBase3<NumType> &source);

#include "lvecBase3.I"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LVecBase3<float>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LVecBase3<double>)

#endif
