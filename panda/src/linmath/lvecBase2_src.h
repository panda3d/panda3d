/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lvecBase2_src.h
 * @author drose
 * @date 2000-03-08
 */

/**
 * This is the base class for all two-component vectors and points.
 */
class EXPCL_PANDA_LINMATH FLOATNAME(LVecBase2) {
PUBLISHED:
  typedef FLOATTYPE numeric_type;
  typedef const FLOATTYPE *iterator;
  typedef const FLOATTYPE *const_iterator;

  enum {
    num_components = 2,

#ifdef FLOATTYPE_IS_INT
    is_int = 1
#else
    is_int = 0
#endif
  };

  INLINE_LINMATH FLOATNAME(LVecBase2)() = default;
  INLINE_LINMATH FLOATNAME(LVecBase2)(FLOATTYPE fill_value);
  INLINE_LINMATH FLOATNAME(LVecBase2)(FLOATTYPE x, FLOATTYPE y);
  ALLOC_DELETED_CHAIN(FLOATNAME(LVecBase2));

#ifdef CPPPARSER
  FLOATNAME(LVecBase2) &operator = (const FLOATNAME(LVecBase2) &copy) = default;
  FLOATNAME(LVecBase2) &operator = (FLOATTYPE fill_value) = default;
#endif

  INLINE_LINMATH static const FLOATNAME(LVecBase2) &zero();
  INLINE_LINMATH static const FLOATNAME(LVecBase2) &unit_x();
  INLINE_LINMATH static const FLOATNAME(LVecBase2) &unit_y();

  EXTENSION(INLINE_LINMATH PyObject *__reduce__(PyObject *self) const);
  EXTENSION(INLINE_LINMATH PyObject *__getattr__(PyObject *self, const std::string &attr_name) const);
  EXTENSION(INLINE_LINMATH int __setattr__(PyObject *self, const std::string &attr_name, PyObject *assign));

  INLINE_LINMATH FLOATTYPE operator [](int i) const;
  INLINE_LINMATH FLOATTYPE &operator [](int i);
  constexpr static int size() { return 2; }

  INLINE_LINMATH bool is_nan() const;

  INLINE_LINMATH FLOATTYPE get_cell(int i) const;
  INLINE_LINMATH void set_cell(int i, FLOATTYPE value);

  INLINE_LINMATH FLOATTYPE get_x() const;
  INLINE_LINMATH FLOATTYPE get_y() const;
  INLINE_LINMATH void set_x(FLOATTYPE value);
  INLINE_LINMATH void set_y(FLOATTYPE value);

PUBLISHED:
  MAKE_PROPERTY(x, get_x, set_x);
  MAKE_PROPERTY(y, get_y, set_y);

  // These next functions add to an existing value.  i.e.
  // foo.set_x(foo.get_x() + value) These are useful to reduce overhead in
  // scripting languages:
  INLINE_LINMATH void add_to_cell(int i, FLOATTYPE value);
  INLINE_LINMATH void add_x(FLOATTYPE value);
  INLINE_LINMATH void add_y(FLOATTYPE value);

  INLINE_LINMATH const FLOATTYPE *get_data() const;
  constexpr static int get_num_components() { return 2; }

public:
  INLINE_LINMATH iterator begin();
  INLINE_LINMATH iterator end();

  INLINE_LINMATH const_iterator begin() const;
  INLINE_LINMATH const_iterator end() const;

PUBLISHED:
  INLINE_LINMATH void fill(FLOATTYPE fill_value);
  INLINE_LINMATH void set(FLOATTYPE x, FLOATTYPE y);

  INLINE_LINMATH FLOATTYPE dot(const FLOATNAME(LVecBase2) &other) const;
  INLINE_LINMATH FLOATTYPE length_squared() const;

#ifndef FLOATTYPE_IS_INT
  INLINE_LINMATH FLOATTYPE length() const;
  INLINE_LINMATH bool normalize();
  INLINE_LINMATH FLOATNAME(LVecBase2) normalized() const;
  INLINE_LINMATH FLOATNAME(LVecBase2) project(const FLOATNAME(LVecBase2) &onto) const;
#endif

  INLINE_LINMATH bool operator < (const FLOATNAME(LVecBase2) &other) const;
  INLINE_LINMATH bool operator == (const FLOATNAME(LVecBase2) &other) const;
  INLINE_LINMATH bool operator != (const FLOATNAME(LVecBase2) &other) const;

  INLINE_LINMATH int compare_to(const FLOATNAME(LVecBase2) &other) const;
  INLINE_LINMATH size_t get_hash() const;
  INLINE_LINMATH size_t add_hash(size_t hash) const;
  INLINE_LINMATH void generate_hash(ChecksumHashGenerator &hashgen) const;

#ifndef FLOATTYPE_IS_INT
  INLINE_LINMATH int compare_to(const FLOATNAME(LVecBase2) &other,
                                FLOATTYPE threshold) const;
  INLINE_LINMATH size_t get_hash(FLOATTYPE threshold) const;
  INLINE_LINMATH size_t add_hash(size_t hash, FLOATTYPE threshold) const;
  INLINE_LINMATH void generate_hash(ChecksumHashGenerator &hashgen,
                                    FLOATTYPE threshold) const;
#endif

  INLINE_LINMATH FLOATNAME(LVecBase2) operator - () const;

  INLINE_LINMATH FLOATNAME(LVecBase2)
  operator + (const FLOATNAME(LVecBase2) &other) const;
  INLINE_LINMATH FLOATNAME(LVecBase2)
  operator - (const FLOATNAME(LVecBase2) &other) const;

  INLINE_LINMATH FLOATNAME(LVecBase2) operator * (FLOATTYPE scalar) const;
  INLINE_LINMATH FLOATNAME(LVecBase2) operator / (FLOATTYPE scalar) const;

  INLINE_LINMATH void operator += (const FLOATNAME(LVecBase2) &other);
  INLINE_LINMATH void operator -= (const FLOATNAME(LVecBase2) &other);

  INLINE_LINMATH void operator *= (FLOATTYPE scalar);
  INLINE_LINMATH void operator /= (FLOATTYPE scalar);

  INLINE_LINMATH void componentwise_mult(const FLOATNAME(LVecBase2) &other);

  EXTENSION(INLINE_LINMATH FLOATNAME(LVecBase2) __pow__(FLOATTYPE exponent) const);
  EXTENSION(INLINE_LINMATH PyObject *__ipow__(PyObject *self, FLOATTYPE exponent));

  INLINE_LINMATH FLOATNAME(LVecBase2) fmax(const FLOATNAME(LVecBase2) &other) const;
  INLINE_LINMATH FLOATNAME(LVecBase2) fmin(const FLOATNAME(LVecBase2) &other) const;

  INLINE_LINMATH bool almost_equal(const FLOATNAME(LVecBase2) &other,
                                   FLOATTYPE threshold) const;
  INLINE_LINMATH bool almost_equal(const FLOATNAME(LVecBase2) &other) const;

  INLINE_LINMATH void output(std::ostream &out) const;
  EXTENSION(INLINE_LINMATH std::string __repr__() const);

  INLINE_LINMATH void write_datagram_fixed(Datagram &destination) const;
  INLINE_LINMATH void read_datagram_fixed(DatagramIterator &source);
  INLINE_LINMATH void write_datagram(Datagram &destination) const;
  INLINE_LINMATH void read_datagram(DatagramIterator &source);

public:
  // The underlying implementation is via the Eigen library, if available.

  // We don't bother to align LVecBase2.  The float version is too small to
  // benefit from SSE2 optimizations.  The double version *would* benefit, but
  // we use this class infrequently throughout the Panda codebase, and the
  // nuisance value of maintaining aligned and unaligned versions of this
  // class outweighs the benefits of having SSE2 optimizations in the
  // stdfloat-double compilation mode.
  typedef UNALIGNED_LINMATH_MATRIX(FLOATTYPE, 1, 2) EVector2;
  EVector2 _v;

  INLINE_LINMATH FLOATNAME(LVecBase2)(const EVector2 &v) : _v(v) { }

private:
  static const FLOATNAME(LVecBase2) _zero;
  static const FLOATNAME(LVecBase2) _unit_x;
  static const FLOATNAME(LVecBase2) _unit_y;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const FLOATNAME(LVecBase2) &vec) {
  vec.output(out);
  return out;
}

#include "lvecBase2_src.I"
