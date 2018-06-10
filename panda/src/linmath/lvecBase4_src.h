/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lvecBase4_src.h
 * @author drose
 * @date 2000-03-08
 */

class FLOATNAME(LVecBase2);
class FLOATNAME(LVecBase3);
class FLOATNAME(LPoint3);
class FLOATNAME(LVector3);
class FLOATNAME(UnalignedLVecBase4);

/**
 * This is the base class for all three-component vectors and points.
 */
class EXPCL_PANDA_LINMATH ALIGN_LINMATH FLOATNAME(LVecBase4) {
PUBLISHED:
  typedef FLOATTYPE numeric_type;
  typedef const FLOATTYPE *iterator;
  typedef const FLOATTYPE *const_iterator;

  enum {
    num_components = 4,

#ifdef FLOATTYPE_IS_INT
    is_int = 1
#else
    is_int = 0
#endif
  };

  INLINE_LINMATH FLOATNAME(LVecBase4)() = default;
  INLINE_LINMATH FLOATNAME(LVecBase4)(FLOATTYPE fill_value);
  INLINE_LINMATH FLOATNAME(LVecBase4)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w);
  INLINE_LINMATH FLOATNAME(LVecBase4)(const FLOATNAME(UnalignedLVecBase4) &copy);
  INLINE_LINMATH FLOATNAME(LVecBase4)(const FLOATNAME(LVecBase3) &copy, FLOATTYPE w);
  INLINE_LINMATH FLOATNAME(LVecBase4)(const FLOATNAME(LPoint3) &point);
  INLINE_LINMATH FLOATNAME(LVecBase4)(const FLOATNAME(LVector3) &vector);
  ALLOC_DELETED_CHAIN(FLOATNAME(LVecBase4));

#ifdef CPPPARSER
  FLOATNAME(LVecBase4) &operator = (const FLOATNAME(LVecBase4) &copy) = default;
  FLOATNAME(LVecBase4) &operator = (FLOATTYPE fill_value) = default;
#endif

  INLINE_LINMATH static const FLOATNAME(LVecBase4) &zero();
  INLINE_LINMATH static const FLOATNAME(LVecBase4) &unit_x();
  INLINE_LINMATH static const FLOATNAME(LVecBase4) &unit_y();
  INLINE_LINMATH static const FLOATNAME(LVecBase4) &unit_z();
  INLINE_LINMATH static const FLOATNAME(LVecBase4) &unit_w();

  EXTENSION(INLINE_LINMATH PyObject *__reduce__(PyObject *self) const);
  EXTENSION(INLINE_LINMATH PyObject *__getattr__(PyObject *self, const std::string &attr_name) const);
  EXTENSION(INLINE_LINMATH int __setattr__(PyObject *self, const std::string &attr_name, PyObject *assign));

  INLINE_LINMATH FLOATTYPE operator [](int i) const;
  INLINE_LINMATH FLOATTYPE &operator [](int i);
  constexpr static int size() { return 4; }

  INLINE_LINMATH bool is_nan() const;

  INLINE_LINMATH FLOATTYPE get_cell(int i) const;
  INLINE_LINMATH void set_cell(int i, FLOATTYPE value);

  INLINE_LINMATH FLOATTYPE get_x() const;
  INLINE_LINMATH FLOATTYPE get_y() const;
  INLINE_LINMATH FLOATTYPE get_z() const;
  INLINE_LINMATH FLOATTYPE get_w() const;

  INLINE_LINMATH FLOATNAME(LVecBase3) get_xyz() const;
  INLINE_LINMATH FLOATNAME(LVecBase2) get_xy() const;

  INLINE_LINMATH void set_x(FLOATTYPE value);
  INLINE_LINMATH void set_y(FLOATTYPE value);
  INLINE_LINMATH void set_z(FLOATTYPE value);
  INLINE_LINMATH void set_w(FLOATTYPE value);

PUBLISHED:
  MAKE_PROPERTY(x, get_x, set_x);
  MAKE_PROPERTY(y, get_y, set_y);
  MAKE_PROPERTY(z, get_z, set_z);

  MAKE_PROPERTY(xyz, get_xyz);
  MAKE_PROPERTY(xy, get_xy);

  // These next functions add to an existing value.  i.e.
  // foo.set_x(foo.get_x() + value) These are useful to reduce overhead in
  // scripting languages:
  INLINE_LINMATH void add_to_cell(int i, FLOATTYPE value);
  INLINE_LINMATH void add_x(FLOATTYPE value);
  INLINE_LINMATH void add_y(FLOATTYPE value);
  INLINE_LINMATH void add_z(FLOATTYPE value);
  INLINE_LINMATH void add_w(FLOATTYPE value);

  INLINE_LINMATH const FLOATTYPE *get_data() const;
  constexpr static int get_num_components() { return 4; }
  INLINE_LINMATH void extract_data(float*){};

public:
  INLINE_LINMATH iterator begin();
  INLINE_LINMATH iterator end();

  INLINE_LINMATH const_iterator begin() const;
  INLINE_LINMATH const_iterator end() const;

PUBLISHED:
  INLINE_LINMATH void fill(FLOATTYPE fill_value);
  INLINE_LINMATH void set(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w);

  INLINE_LINMATH FLOATTYPE dot(const FLOATNAME(LVecBase4) &other) const;
  INLINE_LINMATH FLOATTYPE length_squared() const;

#ifndef FLOATTYPE_IS_INT
  INLINE_LINMATH FLOATTYPE length() const;
  INLINE_LINMATH bool normalize();
  INLINE_LINMATH FLOATNAME(LVecBase4) normalized() const;
  INLINE_LINMATH FLOATNAME(LVecBase4) project(const FLOATNAME(LVecBase4) &onto) const;
#endif

  INLINE_LINMATH bool operator < (const FLOATNAME(LVecBase4) &other) const;
  INLINE_LINMATH bool operator == (const FLOATNAME(LVecBase4) &other) const;
  INLINE_LINMATH bool operator != (const FLOATNAME(LVecBase4) &other) const;

  INLINE_LINMATH int compare_to(const FLOATNAME(LVecBase4) &other) const;
  INLINE_LINMATH size_t get_hash() const;
  INLINE_LINMATH size_t add_hash(size_t hash) const;
  INLINE_LINMATH void generate_hash(ChecksumHashGenerator &hashgen) const;

#ifndef FLOATTYPE_IS_INT
  INLINE_LINMATH int compare_to(const FLOATNAME(LVecBase4) &other,
                                FLOATTYPE threshold) const;
  INLINE_LINMATH size_t get_hash(FLOATTYPE threshold) const;
  INLINE_LINMATH size_t add_hash(size_t hash, FLOATTYPE threshold) const;
  INLINE_LINMATH void generate_hash(ChecksumHashGenerator &hashgen,
                                    FLOATTYPE threshold) const;
#endif

  INLINE_LINMATH FLOATNAME(LVecBase4) operator - () const;

  INLINE_LINMATH FLOATNAME(LVecBase4)
  operator + (const FLOATNAME(LVecBase4) &other) const;
  INLINE_LINMATH FLOATNAME(LVecBase4)
  operator - (const FLOATNAME(LVecBase4) &other) const;

  INLINE_LINMATH FLOATNAME(LVecBase4) operator * (FLOATTYPE scalar) const;
  INLINE_LINMATH FLOATNAME(LVecBase4) operator / (FLOATTYPE scalar) const;

  INLINE_LINMATH void operator += (const FLOATNAME(LVecBase4) &other);
  INLINE_LINMATH void operator -= (const FLOATNAME(LVecBase4) &other);

  INLINE_LINMATH void operator *= (FLOATTYPE scalar);
  INLINE_LINMATH void operator /= (FLOATTYPE scalar);

  INLINE_LINMATH void componentwise_mult(const FLOATNAME(LVecBase4) &other);

  EXTENSION(INLINE_LINMATH FLOATNAME(LVecBase4) __pow__(FLOATTYPE exponent) const);
  EXTENSION(INLINE_LINMATH PyObject *__ipow__(PyObject *self, FLOATTYPE exponent));

  INLINE_LINMATH FLOATNAME(LVecBase4) fmax(const FLOATNAME(LVecBase4) &other) const;
  INLINE_LINMATH FLOATNAME(LVecBase4) fmin(const FLOATNAME(LVecBase4) &other) const;

  INLINE_LINMATH bool almost_equal(const FLOATNAME(LVecBase4) &other,
                                   FLOATTYPE threshold) const;
  INLINE_LINMATH bool almost_equal(const FLOATNAME(LVecBase4) &other) const;

  INLINE_LINMATH void output(std::ostream &out) const;
  EXTENSION(INLINE_LINMATH std::string __repr__() const);

  INLINE_LINMATH void write_datagram_fixed(Datagram &destination) const;
  INLINE_LINMATH void read_datagram_fixed(DatagramIterator &source);
  INLINE_LINMATH void write_datagram(Datagram &destination) const;
  INLINE_LINMATH void read_datagram(DatagramIterator &source);

public:
  // The underlying implementation is via the Eigen library, if available.

  // Unlike LVecBase2 and LVecBase3, we fully align LVecBase4 to 16-byte
  // boundaries, to take advantage of SSE2 optimizations when available.
  // Sometimes this alignment requirement is inconvenient, so we also provide
  // UnalignedLVecBase4, below.
  typedef LINMATH_MATRIX(FLOATTYPE, 1, 4) EVector4;
  EVector4 _v;

  INLINE_LINMATH FLOATNAME(LVecBase4)(const EVector4 &v) : _v(v) { }

private:
  static const FLOATNAME(LVecBase4) _zero;
  static const FLOATNAME(LVecBase4) _unit_x;
  static const FLOATNAME(LVecBase4) _unit_y;
  static const FLOATNAME(LVecBase4) _unit_z;
  static const FLOATNAME(LVecBase4) _unit_w;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

/**
 * This is an "unaligned" LVecBase4.  It has no functionality other than to
 * store numbers, and it will pack them in as tightly as possible, avoiding
 * any SSE2 alignment requirements shared by the primary LVecBase4 class.
 *
 * Use it only when you need to pack numbers tightly without respect to
 * alignment, and then copy it to a proper LVecBase4 to get actual use from
 * it.
 */
class EXPCL_PANDA_LINMATH FLOATNAME(UnalignedLVecBase4) {
PUBLISHED:
  enum {
    num_components = 4,

#ifdef FLOATTYPE_IS_INT
    is_int = 1
#else
    is_int = 0
#endif
  };

  INLINE_LINMATH FLOATNAME(UnalignedLVecBase4)() = default;
  INLINE_LINMATH FLOATNAME(UnalignedLVecBase4)(const FLOATNAME(LVecBase4) &copy);
  INLINE_LINMATH FLOATNAME(UnalignedLVecBase4)(FLOATTYPE fill_value);
  INLINE_LINMATH FLOATNAME(UnalignedLVecBase4)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w);

  INLINE_LINMATH void fill(FLOATTYPE fill_value);
  INLINE_LINMATH void set(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w);

  INLINE_LINMATH FLOATTYPE operator [](int i) const;
  INLINE_LINMATH FLOATTYPE &operator [](int i);
  constexpr static int size() { return 4; }

  INLINE_LINMATH const FLOATTYPE *get_data() const;
  constexpr static int get_num_components() { return 4; }

  INLINE_LINMATH bool operator == (const FLOATNAME(UnalignedLVecBase4) &other) const;
  INLINE_LINMATH bool operator != (const FLOATNAME(UnalignedLVecBase4) &other) const;

public:
  typedef FLOATTYPE numeric_type;
  typedef UNALIGNED_LINMATH_MATRIX(FLOATTYPE, 1, 4) UVector4;
  UVector4 _v;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const FLOATNAME(LVecBase4) &vec) {
  vec.output(out);
  return out;
}

#include "lvecBase4_src.I"
