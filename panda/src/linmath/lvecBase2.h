// Filename: lvecBase2.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LVECBASE2_H
#define LVECBASE2_H

#include <pandabase.h>

#include "cmath.h"

#include <typeHandle.h>

class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
// 	 Class : LVecBase2
// Description : This is the base class for all two-component
//               vectors and points.
////////////////////////////////////////////////////////////////////
template<class NumType>
class LVecBase2 {
PUBLISHED:
  typedef const NumType *iterator;
  typedef const NumType *const_iterator;

  INLINE LVecBase2();
  INLINE LVecBase2(const LVecBase2<NumType> &copy);
  INLINE LVecBase2<NumType> &operator = (const LVecBase2<NumType> &copy);
  INLINE LVecBase2<NumType> &operator = (NumType fill_value);
  INLINE LVecBase2(NumType fill_value);
  INLINE LVecBase2(NumType x, NumType y);

  INLINE static LVecBase2<NumType> zero();
  INLINE static LVecBase2<NumType> unit_x();
  INLINE static LVecBase2<NumType> unit_y();

  INLINE ~LVecBase2();

  INLINE NumType operator [](int i) const;
  INLINE NumType &operator [](int i);

  INLINE bool is_nan() const;

  INLINE NumType get_cell(int i) const;
  INLINE NumType get_x() const;
  INLINE NumType get_y() const;
  INLINE void set_cell(int i, NumType value);
  INLINE void set_x(NumType value);
  INLINE void set_y(NumType value);

  INLINE const NumType *get_data() const;
  INLINE int get_num_components() const;

public:
  INLINE iterator begin();
  INLINE iterator end();

  INLINE const_iterator begin() const;
  INLINE const_iterator end() const;

PUBLISHED:
  INLINE void fill(NumType fill_value);
  INLINE void set(NumType x, NumType y);

  INLINE NumType dot(const LVecBase2<NumType> &other) const;

  INLINE bool operator < (const LVecBase2<NumType> &other) const;
  INLINE bool operator == (const LVecBase2<NumType> &other) const;
  INLINE bool operator != (const LVecBase2<NumType> &other) const;

  INLINE int compare_to(const LVecBase2<NumType> &other) const;
  INLINE int compare_to(const LVecBase2<NumType> &other,
		        NumType threshold) const;

  INLINE LVecBase2<NumType> 
  operator - () const;

  INLINE LVecBase2<NumType>
  operator + (const LVecBase2<NumType> &other) const;
  INLINE LVecBase2<NumType>
  operator - (const LVecBase2<NumType> &other) const;

  INLINE LVecBase2<NumType> operator * (NumType scalar) const;
  INLINE LVecBase2<NumType> operator / (NumType scalar) const;

  INLINE void operator += (const LVecBase2<NumType> &other);
  INLINE void operator -= (const LVecBase2<NumType> &other);

  INLINE void operator *= (NumType scalar);
  INLINE void operator /= (NumType scalar);

  INLINE bool almost_equal(const LVecBase2<NumType> &other, 
			   NumType threshold) const;
  INLINE bool almost_equal(const LVecBase2<NumType> &other) const;

  INLINE void output(ostream &out) const;

private:
  NumType _data[2];

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
INLINE ostream &operator << (ostream &out, const LVecBase2<NumType> &vec) {
  vec.output(out);
  return out;
}


// Cast to a different numeric type
template<class NumType, class NumType2>
INLINE LVecBase2<NumType2> 
lcast_to(NumType2 *type, const LVecBase2<NumType> &source);

#include "lvecBase2.I"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LVecBase2<float>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LVecBase2<double>)

#endif
