// Filename: lvecBase2_src.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// 	 Class : LVecBase2
// Description : This is the base class for all two-component
//               vectors and points.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LVecBase2) {
PUBLISHED:
  typedef const FLOATTYPE *iterator;
  typedef const FLOATTYPE *const_iterator;

  INLINE FLOATNAME(LVecBase2)();
  INLINE FLOATNAME(LVecBase2)(const FLOATNAME(LVecBase2) &copy);
  INLINE FLOATNAME(LVecBase2) &operator = (const FLOATNAME(LVecBase2) &copy);
  INLINE FLOATNAME(LVecBase2) &operator = (FLOATTYPE fill_value);
  INLINE FLOATNAME(LVecBase2)(FLOATTYPE fill_value);
  INLINE FLOATNAME(LVecBase2)(FLOATTYPE x, FLOATTYPE y);

  INLINE static const FLOATNAME(LVecBase2) &zero();
  INLINE static const FLOATNAME(LVecBase2) &unit_x();
  INLINE static const FLOATNAME(LVecBase2) &unit_y();

  INLINE ~FLOATNAME(LVecBase2)();

  INLINE FLOATTYPE operator [](int i) const;
  INLINE FLOATTYPE &operator [](int i);

  INLINE bool is_nan() const;

  INLINE FLOATTYPE get_cell(int i) const;
  INLINE FLOATTYPE get_x() const;
  INLINE FLOATTYPE get_y() const;
  INLINE void set_cell(int i, FLOATTYPE value);
  INLINE void set_x(FLOATTYPE value);
  INLINE void set_y(FLOATTYPE value);

  INLINE const FLOATTYPE *get_data() const;
  INLINE int get_num_components() const;

public:
  INLINE iterator begin();
  INLINE iterator end();

  INLINE const_iterator begin() const;
  INLINE const_iterator end() const;

PUBLISHED:
  INLINE void fill(FLOATTYPE fill_value);
  INLINE void set(FLOATTYPE x, FLOATTYPE y);

  INLINE FLOATTYPE dot(const FLOATNAME(LVecBase2) &other) const;

  INLINE bool operator < (const FLOATNAME(LVecBase2) &other) const;
  INLINE bool operator == (const FLOATNAME(LVecBase2) &other) const;
  INLINE bool operator != (const FLOATNAME(LVecBase2) &other) const;

  INLINE int compare_to(const FLOATNAME(LVecBase2) &other) const;
  INLINE int compare_to(const FLOATNAME(LVecBase2) &other,
		        FLOATTYPE threshold) const;

  INLINE FLOATNAME(LVecBase2) operator - () const;

  INLINE FLOATNAME(LVecBase2)
  operator + (const FLOATNAME(LVecBase2) &other) const;
  INLINE FLOATNAME(LVecBase2)
  operator - (const FLOATNAME(LVecBase2) &other) const;

  INLINE FLOATNAME(LVecBase2) operator * (FLOATTYPE scalar) const;
  INLINE FLOATNAME(LVecBase2) operator / (FLOATTYPE scalar) const;

  INLINE void operator += (const FLOATNAME(LVecBase2) &other);
  INLINE void operator -= (const FLOATNAME(LVecBase2) &other);

  INLINE void operator *= (FLOATTYPE scalar);
  INLINE void operator /= (FLOATTYPE scalar);

  INLINE bool almost_equal(const FLOATNAME(LVecBase2) &other, 
			   FLOATTYPE threshold) const;
  INLINE bool almost_equal(const FLOATNAME(LVecBase2) &other) const;

  INLINE void output(ostream &out) const;

protected:
  FLOATTYPE _data[2];

private:
  static const FLOATNAME(LVecBase2) _zero;
  static const FLOATNAME(LVecBase2) _unit_x;
  static const FLOATNAME(LVecBase2) _unit_y;

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


INLINE ostream &operator << (ostream &out, const FLOATNAME(LVecBase2) &vec) {
  vec.output(out);
  return out;
}

#include "lvecBase2_src.I"
