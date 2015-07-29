// Filename: parameter.h
// Created by:  drose (08Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PARAMETER_H
#define PARAMETER_H

#include "pandabase.h"

#include "typedef.h"
#include "typedObject.h"
#include "typedReferenceCount.h"
#include "typedWritableReferenceCount.h"
#include "paramValue.h"
#include "pointerTo.h"
#include "extension.h"

////////////////////////////////////////////////////////////////////
//       Class : Parameter
// Description : A generic parameter object that may be used as a
//               shader input or an event parameter.  Each parameter
//               stores either a primitive type or a pointer to a
//               TypedWritableReferenceCount or TypedReferenceCount
//               object, which of course could be pretty much
//               anything.
//
//               This class is optimized to be able to be passed
//               around on the stack as it is only 8 bytes.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_EVENT Parameter {
PUBLISHED:
  INLINE Parameter();
  //EXTENSION(INLINE Parameter(PyObject *value));

  friend class Extension<Parameter>;

public:
  INLINE Parameter(const TypedWritableReferenceCount *ptr);
  INLINE Parameter(const TypedReferenceCount *ptr);
  INLINE Parameter(int value);
  INLINE Parameter(float value);
  INLINE Parameter(double value);
  INLINE Parameter(const LVecBase2i &value);
  INLINE Parameter(const LVecBase2f &value);
  INLINE Parameter(const LVecBase2d &value);
  INLINE Parameter(const LVecBase3i &value);
  INLINE Parameter(const LVecBase3f &value);
  INLINE Parameter(const LVecBase3d &value);
  INLINE Parameter(const LVecBase4i &value);
  INLINE Parameter(const LVecBase4f &value);
  INLINE Parameter(const LVecBase4d &value);
  INLINE Parameter(const LPoint3f &value);
  INLINE Parameter(const LPoint3d &value);
  INLINE Parameter(const LVector3f &value);
  INLINE Parameter(const LVector3d &value);
  INLINE Parameter(const LMatrix3f &value);
  INLINE Parameter(const LMatrix3d &value);
  INLINE Parameter(const LMatrix4f &value);
  INLINE Parameter(const LMatrix4d &value);
  INLINE Parameter(const string &value);
  INLINE Parameter(const wstring &value);

  INLINE Parameter(const Parameter &copy);
  INLINE Parameter &operator = (const Parameter &copy);
  INLINE ~Parameter();

#ifdef USE_MOVE_SEMANTICS
  INLINE Parameter(Parameter &&from);
  INLINE Parameter &operator = (Parameter &&from);
#endif

PUBLISHED:
  INLINE void clear();
  INLINE bool is_empty() const;

public:
  INLINE void swap(Parameter &other) NOEXCEPT;

  INLINE void set_value(const TypedWritableReferenceCount *ptr);
  INLINE void set_value(const TypedReferenceCount *ptr);
  INLINE void set_value(int value);
  INLINE void set_value(bool value);
  INLINE void set_value(float value);
  INLINE void set_value(double value);
  INLINE void set_value(const LVecBase3f &value);

PUBLISHED:
  EXTENSION(void set_value(PyObject *value));
  EXTENSION(PyObject *get_value() const);

  MAKE_PROPERTY(value, get_value, set_value);

  INLINE TypeHandle get_value_type() const;

  // These functions are conveniences to easily determine if the
  // Parameter is one of the predefined parameter types, and
  // retrieve the corresponding value.  Of course, it is possible that
  // the Parameter is some user-defined type, and is none of
  // these.
  INLINE bool is_int() const;
  INLINE int get_int_value() const;
  INLINE bool is_float() const;
  INLINE float get_float_value() const;
  INLINE bool is_double() const;
  INLINE double get_double_value() const;

  INLINE bool is_string() const;
  INLINE const string &get_string_value() const;
  INLINE bool is_wstring() const;
  INLINE const wstring &get_wstring_value() const;

  INLINE bool is_typed_ref_count() const;
  INLINE TypedReferenceCount *get_typed_ref_count_value() const;

  INLINE TypedObject *get_typed_object() const;

  void output(ostream &out) const;

  INLINE void write_datagram(BamWriter *manager, Datagram &destination) const;
  INLINE void read_datagram(DatagramIterator &source, BamReader *manager);
  INLINE int complete_pointers(TypedWritable **plist, BamReader *manager);

public:
  // These functions are used by the graphics back-end to extract
  // underlying numeric data.  They may convert data if appropriate.
  bool extract_data(int *data, int width, size_t count) const;
  bool extract_data(float *data, int width, size_t count) const;
  bool extract_data(double *data, int width, size_t count) const;

private:
  INLINE double do_get_double() const;
  INLINE void do_set_double(double value);

  INLINE ReferenceCount *do_get_ptr() const;
  INLINE void do_set_ptr(ReferenceCount *ptr);

  // I used a classic pointer-preferred NaN-boxing implementation
  // to avoid the overhead of using a ParamValue for the simple
  // types.  This means that this class can hold doubles and
  // pointers while still taking up only 8 bytes on the stack.
  //
  // All known 64-bit platforms only use the lower 48 bits for
  // (most) user-space addressing, which perfectly overlaps with
  // NaN-payload space of IEEE-754 doubles.  So, we invert the
  // exponent space of doubles and extend it with a type indicator.
  //
  // Some day, another developer is going to find this and
  // subject me to some well-deserved flogging.
  enum Type {
    T_pointer = 0x00000000,
    T_int     = 0x00010000,
    T_bool    = 0x00020000,
    T_float   = 0x00030000,
    T_MAX     = T_float,

    // These are used to mask out the enum bits in case that
    // they overlap with that of the pointer on 64-bit systems.
    // Flipping the type values by T_MASK also results in a
    // non-canonical NaN representation so that we can distinguish
    // it easily from an actual double value.
    T_MASK    = 0x7fff0000,
  };

  INLINE Type do_get_type() const;

  union Value {
    // This is either a TypedWritableReferenceCount or a
    // TypedReferenceCount, but we have to pick a common base.
    //
    // Note: C++ doesn't allow us to put a PointerTo in a union,
    // so we have to manage the reference count ourselves.
#if NATIVE_WORDSIZE == 64
    ReferenceCount *_ptr;
#endif
    double _double;

    struct Packed {
#ifdef WORDS_BIGENDIAN
      PN_uint32 _type;
#endif

      union {
        bool _bool;
        float _float;
        int _int;
#if NATIVE_WORDSIZE != 64
        // On a 32-bits system, pointers don't have to overlap with
        // the type enum, since they fit comfortably in this union.
        ReferenceCount *_ptr;
#endif
      };

#ifndef WORDS_BIGENDIAN
      PN_uint32 _type;
#endif
    } _packed;
  } _v;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "Parameter");
  }

private:
  static TypeHandle _type_handle;
};

nassert_static(sizeof(Parameter) == 8);

INLINE void swap(Parameter &one, Parameter &two) NOEXCEPT;
INLINE ostream &operator << (ostream &out, const Parameter &param);

#include "parameter.I"

#endif
