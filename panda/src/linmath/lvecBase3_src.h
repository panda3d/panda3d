// Filename: lvecBase3_src.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// 	 Class : LVecBase3
// Description : This is the base class for all three-component
//               vectors and points.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LVecBase3) {
PUBLISHED:
  typedef const FLOATTYPE *iterator;
  typedef const FLOATTYPE *const_iterator;

  INLINE FLOATNAME(LVecBase3)();
  INLINE FLOATNAME(LVecBase3)(const FLOATNAME(LVecBase3) &copy);
  INLINE FLOATNAME(LVecBase3) &operator = (const FLOATNAME(LVecBase3) &copy);
  INLINE FLOATNAME(LVecBase3) &operator = (FLOATTYPE fill_value);
  INLINE FLOATNAME(LVecBase3)(FLOATTYPE fill_value);
  INLINE FLOATNAME(LVecBase3)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z);

  INLINE static const FLOATNAME(LVecBase3) &zero();
  INLINE static const FLOATNAME(LVecBase3) &unit_x();
  INLINE static const FLOATNAME(LVecBase3) &unit_y();
  INLINE static const FLOATNAME(LVecBase3) &unit_z();

  INLINE ~FLOATNAME(LVecBase3)();

  INLINE FLOATTYPE operator [](int i) const;
  INLINE FLOATTYPE &operator [](int i);

  INLINE bool is_nan() const;

  INLINE FLOATTYPE get_cell(int i) const;
  INLINE FLOATTYPE get_x() const;
  INLINE FLOATTYPE get_y() const;
  INLINE FLOATTYPE get_z() const;
  INLINE void set_cell(int i, FLOATTYPE value);
  INLINE void set_x(FLOATTYPE value);
  INLINE void set_y(FLOATTYPE value);
  INLINE void set_z(FLOATTYPE value);

  INLINE const FLOATTYPE *get_data() const;
  INLINE int get_num_components() const;

public:
  INLINE iterator begin();
  INLINE iterator end();

  INLINE const_iterator begin() const;
  INLINE const_iterator end() const;

PUBLISHED:
  INLINE void fill(FLOATTYPE fill_value);
  INLINE void set(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z);

  INLINE FLOATTYPE dot(const FLOATNAME(LVecBase3) &other) const;
  INLINE FLOATNAME(LVecBase3) cross(const FLOATNAME(LVecBase3) &other) const;

  INLINE bool operator < (const FLOATNAME(LVecBase3) &other) const;
  INLINE bool operator == (const FLOATNAME(LVecBase3) &other) const;
  INLINE bool operator != (const FLOATNAME(LVecBase3) &other) const;

  INLINE int compare_to(const FLOATNAME(LVecBase3) &other) const;
  INLINE int compare_to(const FLOATNAME(LVecBase3) &other,
		        FLOATTYPE threshold) const;

  INLINE FLOATNAME(LVecBase3) operator - () const;

  INLINE FLOATNAME(LVecBase3)
  operator + (const FLOATNAME(LVecBase3) &other) const;
  INLINE FLOATNAME(LVecBase3)
  operator - (const FLOATNAME(LVecBase3) &other) const;

  INLINE FLOATNAME(LVecBase3) operator * (FLOATTYPE scalar) const;
  INLINE FLOATNAME(LVecBase3) operator / (FLOATTYPE scalar) const;

  INLINE void operator += (const FLOATNAME(LVecBase3) &other);
  INLINE void operator -= (const FLOATNAME(LVecBase3) &other);

  INLINE void operator *= (FLOATTYPE scalar);
  INLINE void operator /= (FLOATTYPE scalar);

  INLINE void cross_into(const FLOATNAME(LVecBase3) &other);

  INLINE bool almost_equal(const FLOATNAME(LVecBase3) &other, 
			   FLOATTYPE threshold) const;
  INLINE bool almost_equal(const FLOATNAME(LVecBase3) &other) const;

  INLINE void output(ostream &out) const;

protected:
  FLOATTYPE _data[3];

private:
  static const FLOATNAME(LVecBase3) _zero;
  static const FLOATNAME(LVecBase3) _unit_x;
  static const FLOATNAME(LVecBase3) _unit_y;
  static const FLOATNAME(LVecBase3) _unit_z;

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


INLINE ostream &operator << (ostream &out, const FLOATNAME(LVecBase3) &vec) {
  vec.output(out);
  return out;
};

#include "lvecBase3_src.I"
