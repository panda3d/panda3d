// Filename: parameter.I
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


////////////////////////////////////////////////////////////////////
//     Function: Parameter::Default constructor
//       Access: Published
//  Description: Defines a Parameter that stores nothing: the
//               "empty" parameter, ie. a NULL pointer.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter() {
  // The empty parameter is a NULL pointer.
  _v._packed._type = T_pointer;

#if NATIVE_WORDSIZE == 64
  _v._ptr = NULL;
#else
  _v._packed._ptr = NULL;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Pointer constructor
//       Access: Published
//  Description: Defines a Parameter that stores a pointer to
//               any kind of TypedWritableReferenceCount object.  This
//               is the most general constructor.
//
//               This accepts a const pointer, even though it stores
//               (and eventually returns) a non-const pointer.  This
//               is just the simplest way to allow both const and
//               non-const pointers to be stored, but it does lose the
//               constness.  Be careful.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const TypedWritableReferenceCount *ptr) {
  do_set_ptr((TypedWritableReferenceCount *)ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Pointer constructor
//       Access: Published
//  Description: Defines a Parameter that stores a pointer to
//               a TypedReferenceCount object.  Note that a
//               TypedReferenceCount is not the same kind of pointer
//               as a TypedWritableReferenceCount, hence we require
//               both constructors.
//
//               This accepts a const pointer, even though it stores
//               (and eventually returns) a non-const pointer.  This
//               is just the simplest way to allow both const and
//               non-const pointers to be stored, but it does lose the
//               constness.  Be careful.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const TypedReferenceCount *ptr) {
  do_set_ptr((TypedReferenceCount *)ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Integer constructor
//       Access: Published
//  Description: Defines a Parameter that stores an integer
//               value.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(int value) {
  _v._packed._int = value;
  _v._packed._type = T_int;
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Float constructor
//       Access: Published
//  Description: Defines a Parameter that stores a
//               floating-point value.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(float value) {
  _v._packed._float = value;
  _v._packed._type = T_float;
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Double constructor
//       Access: Published
//  Description: Defines a Parameter that stores a
//               double-precision floating-point value.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(double value) {
  do_set_double(value);
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Vector constructor
//       Access: Published
//  Description: Defines a Parameter that stores a vector.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const LVecBase2i &value) {
  do_set_ptr(new ParamVecBase2i(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Vector constructor
//       Access: Published
//  Description: Defines a Parameter that stores a vector.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const LVecBase2f &value) {
  do_set_ptr(new ParamVecBase2f(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Vector constructor
//       Access: Published
//  Description: Defines a Parameter that stores a vector.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const LVecBase2d &value) {
  do_set_ptr(new ParamVecBase2d(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Vector constructor
//       Access: Published
//  Description: Defines a Parameter that stores a vector.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const LVecBase3i &value) {
  do_set_ptr(new ParamVecBase3i(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Vector constructor
//       Access: Published
//  Description: Defines a Parameter that stores a vector.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const LVecBase3f &value) {
  do_set_ptr(new ParamVecBase3f(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Vector constructor
//       Access: Published
//  Description: Defines a Parameter that stores a vector.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const LVecBase3d &value) {
  do_set_ptr(new ParamVecBase3d(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Vector constructor
//       Access: Published
//  Description: Defines a Parameter that stores a vector.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const LVecBase4i &value) {
  do_set_ptr(new ParamVecBase4i(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Vector constructor
//       Access: Published
//  Description: Defines a Parameter that stores a vector.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const LVecBase4f &value) {
  do_set_ptr(new ParamVecBase4f(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Vector constructor
//       Access: Published
//  Description: Defines a Parameter that stores a vector.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const LVecBase4d &value) {
  do_set_ptr(new ParamVecBase4d(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Vector constructor
//       Access: Published
//  Description: Defines a Parameter that stores a point.  Actually
//               stores it as an LVecBase4 with the w coordinate
//               set to 1.0.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const LPoint3f &value) {
  do_set_ptr(new ParamVecBase4f(LVecBase4f(value)));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Vector constructor
//       Access: Published
//  Description: Defines a Parameter that stores a point.  Actually
//               stores it as an LVecBase4 with the w coordinate
//               set to 1.0.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const LPoint3d &value) {
  do_set_ptr(new ParamVecBase4d(LVecBase4d(value)));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Vector constructor
//       Access: Published
//  Description: Defines a Parameter that stores a vector.  Actually
//               stores it as an LVecBase4 with the w coordinate
//               set to 0.0.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const LVector3f &value) {
  do_set_ptr(new ParamVecBase4f(LVecBase4f(value)));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Vector constructor
//       Access: Published
//  Description: Defines a Parameter that stores a vector.  Actually
//               stores it as an LVecBase4 with the w coordinate
//               set to 0.0.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const LVector3d &value) {
  do_set_ptr(new ParamVecBase4d(LVecBase4d(value)));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Matrix constructor
//       Access: Published
//  Description: Defines a Parameter that stores a matrix.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const LMatrix3f &value) {
  do_set_ptr(new ParamMatrix3f(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Matrix constructor
//       Access: Published
//  Description: Defines a Parameter that stores a matrix.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const LMatrix3d &value) {
  do_set_ptr(new ParamMatrix3d(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Matrix constructor
//       Access: Published
//  Description: Defines a Parameter that stores a matrix.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const LMatrix4f &value) {
  do_set_ptr(new ParamMatrix4f(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Matrix constructor
//       Access: Published
//  Description: Defines a Parameter that stores a matrix.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const LMatrix4d &value) {
  do_set_ptr(new ParamMatrix4d(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::String constructor
//       Access: Published
//  Description: Defines a Parameter that stores a string value.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const string &value) {
  do_set_ptr(new ParamString(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Wstring constructor
//       Access: Published
//  Description: Defines a Parameter that stores a wstring value.
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const wstring &value) {
  do_set_ptr(new ParamWstring(value));
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Copy constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(const Parameter &other) :
  _v(other._v) {

  if (do_get_type() == T_pointer &&
      do_get_ptr() != (ReferenceCount *)NULL) {
    do_get_ptr()->ref();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Copy assignment operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Parameter &Parameter::
operator = (const Parameter &other) {
  if (do_get_type() == T_pointer &&
      do_get_ptr() != (ReferenceCount *)NULL) {
    if ((other.do_get_type()) == T_pointer && do_get_ptr() == other.do_get_ptr()){
      // The new pointer is the same as the old, so don't do anything.
      return *this;
    }
    unref_delete(do_get_ptr());
  }

  _v = other._v;

  if (do_get_type() == T_pointer &&
      do_get_ptr() != (ReferenceCount *)NULL) {
    do_get_ptr()->ref();
  }

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Parameter::
~Parameter() {
  if (do_get_type() == T_pointer &&
      do_get_ptr() != (ReferenceCount *)NULL) {
    // Dereference the assigned pointer, if we had one.
    unref_delete(do_get_ptr());
  }
}

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: Parameter::Move constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Parameter::
Parameter(Parameter &&from) :
  _v(from._v) {

  // Clear the other pointer.
  from._v._packed._type = T_pointer;

#if NATIVE_WORDSIZE == 64
  from._v._ptr = NULL;
#else
  from._v._packed._ptr = NULL;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::Move assignment operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Parameter &Parameter::
operator = (Parameter &&from) {
  if (do_get_type() == T_pointer &&
      do_get_ptr() != (ReferenceCount *)NULL) {
    // If we had a pointer, dereference it.
    unref_delete(do_get_ptr());
  }

  _v = from._v;
  from._v._packed._type = T_pointer;

#if NATIVE_WORDSIZE == 64
  from._v._ptr = NULL;
#else
  from._v._packed._ptr = NULL;
#endif

  return *this;
}
#endif  // USE_MOVE_SEMANTICS

////////////////////////////////////////////////////////////////////
//     Function: Parameter::clear
//       Access: Published
//  Description: Clears the parameter (equivalent to setting it to
//               a NULL/None pointer).  is_empty() will return true.
////////////////////////////////////////////////////////////////////
INLINE void Parameter::
clear() {
  if (do_get_type() == T_pointer &&
      do_get_ptr() != (ReferenceCount *)NULL) {
    unref_delete(do_get_ptr());
  }

  do_set_ptr((ReferenceCount *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::is_empty
//       Access: Published
//  Description: Returns true if the Parameter is the empty
//               parameter, storing nothing, or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Parameter::
is_empty() const {
  return (do_get_type() == T_pointer &&
          do_get_ptr() == (ReferenceCount *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::swap
//       Access: Published
//  Description: Swaps the value of this parameter with the other.
////////////////////////////////////////////////////////////////////
INLINE void Parameter::
swap(Parameter &other) NOEXCEPT {
  Value temp = other._v;
  other._v = _v;
  _v = temp;
}

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: Parameter::set_value
//       Access: Published
//  Description: Changes the value associated with this Parameter.
////////////////////////////////////////////////////////////////////
INLINE void Parameter::
set_value(const TypedWritableReferenceCount *value) {
  if (do_get_type() == T_pointer &&
      do_get_ptr() != (ReferenceCount *)NULL) {
    if (do_get_ptr() == value){
      return;
    }
    unref_delete(do_get_ptr());
  }

  do_set_ptr((TypedWritableReferenceCount*) value);
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::set_value
//       Access: Published
//  Description: Changes the value associated with this Parameter.
////////////////////////////////////////////////////////////////////
INLINE void Parameter::
set_value(const TypedReferenceCount *value) {
  if (do_get_type() == T_pointer &&
      do_get_ptr() != (ReferenceCount *)NULL) {
    if (do_get_ptr() == value){
      return;
    }
    unref_delete(do_get_ptr());
  }

  do_set_ptr((TypedReferenceCount*) value);
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::set_value
//       Access: Published
//  Description: Changes the value associated with this Parameter.
////////////////////////////////////////////////////////////////////
INLINE void Parameter::
set_value(int value) {
  if (do_get_type() == T_pointer &&
      do_get_ptr() != (ReferenceCount *)NULL) {
    unref_delete(do_get_ptr());
  }

  _v._packed._type = T_int;
  _v._packed._int = value;
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::set_value
//       Access: Published
//  Description: Changes the value associated with this Parameter.
////////////////////////////////////////////////////////////////////
INLINE void Parameter::
set_value(bool value) {
  if (do_get_type() == T_pointer &&
      do_get_ptr() != (ReferenceCount *)NULL) {
    unref_delete(do_get_ptr());
  }

  _v._packed._type = T_bool;
  _v._packed._bool = value;
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::set_value
//       Access: Published
//  Description: Changes the value associated with this Parameter.
////////////////////////////////////////////////////////////////////
INLINE void Parameter::
set_value(float value) {
  if (do_get_type() == T_pointer &&
      do_get_ptr() != (ReferenceCount *)NULL) {
    unref_delete(do_get_ptr());
  }

  _v._packed._type = T_float;
  _v._packed._float = value;
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::set_value
//       Access: Published
//  Description: Changes the value associated with this Parameter.
////////////////////////////////////////////////////////////////////
INLINE void Parameter::
set_value(double value) {
  if (do_get_type() == T_pointer &&
      do_get_ptr() != (ReferenceCount *)NULL) {
    unref_delete(do_get_ptr());
  }

  do_set_double(value);
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::set_value
//       Access: Published
//  Description: Changes the value associated with this Parameter.
////////////////////////////////////////////////////////////////////
INLINE void Parameter::
set_value(const LVecBase3f &value) {
  set_value(new ParamVecBase3f(value));
}
#endif  // CPPPARSER

////////////////////////////////////////////////////////////////////
//     Function: Parameter::get_value_type
//       Access: Published
//  Description: Returns the TypeHandle corresponding to the type
//               of this parameter's value.
////////////////////////////////////////////////////////////////////
INLINE TypeHandle Parameter::
get_value_type() const {
  switch (do_get_type()) {
  case T_pointer:
    {
      TypedObject *obj = get_typed_object();
      if (obj != (TypedObject *)NULL) {
        return obj->get_type();
      }
    }
    break;

  case T_int:
    return int_type_handle;

  case T_bool:
    return bool_type_handle;

  case T_float:
    return float_type_handle;

  default:
    return double_type_handle;
  }

  return TypeHandle::none();
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::is_int
//       Access: Published
//  Description: Returns true if the Parameter stores an integer
//               value, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Parameter::
is_int() const {
  return (do_get_type() == T_int);
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::get_int_value
//       Access: Published
//  Description: Retrieves the value stored in the Parameter.  It
//               is only valid to call this if is_int() has already
//               returned true.
////////////////////////////////////////////////////////////////////
INLINE int Parameter::
get_int_value() const {
  nassertr(is_int(), 0);
  return _v._packed._int;
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::is_float
//       Access: Published
//  Description: Returns true if the Parameter stores a single
//               floating-point value, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Parameter::
is_float() const {
  return (do_get_type() == T_float);
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::get_float_value
//       Access: Published
//  Description: Retrieves the value stored in the Parameter.  If it
//               is of a different numeric type, it will be converted.
//               It is an error to call this on non-numeric parameters.
////////////////////////////////////////////////////////////////////
INLINE float Parameter::
get_float_value() const {
  switch (do_get_type()) {
  case T_float:
    return _v._packed._float;

  case T_int:
    return (float) _v._packed._int;

  case T_bool:
    return (float) (int) _v._packed._bool;

  case T_pointer:
    nassertr(do_get_type() != T_pointer, do_get_double());
    // Fall through - do_get_double() will produce NaN

  default:
    return (float) do_get_double();
  }
  return 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::is_double
//       Access: Published
//  Description: Returns true if the Parameter stores a single
//               double-precision float, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Parameter::
is_double() const {
  return (do_get_type() > T_MAX);
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::get_double_value
//       Access: Published
//  Description: Retrieves the value stored in the Parameter.  If it
//               is of a different numeric type, it will be converted.
//               It is an error to call this on non-numeric parameters.
////////////////////////////////////////////////////////////////////
INLINE double Parameter::
get_double_value() const {
  switch (do_get_type()) {
  case T_float:
    return (double) _v._packed._float;

  case T_int:
    return (double) _v._packed._int;

  case T_bool:
    return (double) (int) _v._packed._bool;

  case T_pointer:
    nassertr(do_get_type() != T_pointer, do_get_double());
    // Fall through - do_get_double() will produce NaN

  default:
    return do_get_double();
  }
  return 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::is_string
//       Access: Published
//  Description: Returns true if the Parameter stores a string
//               value, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Parameter::
is_string() const {
  if (do_get_type() != T_pointer || do_get_ptr() == (ReferenceCount *)NULL){
    return false;
  }
  return (get_typed_object()->get_type() == ParamString::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::get_string_value
//       Access: Published
//  Description: Retrieves the value stored in the Parameter.  It
//               is only valid to call this if is_string() has already
//               returned true.
////////////////////////////////////////////////////////////////////
INLINE const string &Parameter::
get_string_value() const {
  static const string empty_string;
  nassertr(is_string(), empty_string);
  // We can't use DCAST, because ParamValue::init_type() breaks
  // convention and takes a parameter.  But the above assertion should
  // protect us.
  return ((const ParamString *)do_get_ptr())->get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::is_wstring
//       Access: Published
//  Description: Returns true if the Parameter stores a wstring
//               value, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Parameter::
is_wstring() const {
  if (do_get_type() != T_pointer || do_get_ptr() == (ReferenceCount *)NULL){
    return false;
  }
  return (get_typed_object()->get_type() == ParamWstring::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::get_wstring_value
//       Access: Published
//  Description: Retrieves the value stored in the Parameter.  It
//               is only valid to call this if is_wstring() has already
//               returned true.
////////////////////////////////////////////////////////////////////
INLINE const wstring &Parameter::
get_wstring_value() const {
  static const wstring empty_wstring;
  nassertr(is_wstring(), empty_wstring);
  return ((const ParamWstring *)do_get_ptr())->get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::is_typed_ref_count
//       Access: Published
//  Description: Returns true if the Parameter stores a
//               TypedReferenceCount pointer, false otherwise.  Note
//               that TypedReferenceCount is not a base class of
//               TypedWritableReferenceCount, hence the need for
//               this separate call.
////////////////////////////////////////////////////////////////////
INLINE bool Parameter::
is_typed_ref_count() const {
  if (do_get_type() != T_pointer || do_get_ptr() == (ReferenceCount *)NULL){
    return false;
  }
  return get_typed_object()->is_of_type(TypedReferenceCount::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::get_typed_ref_count_value
//       Access: Published
//  Description: Retrieves the value stored in the Parameter.  It
//               is only valid to call this if is_typed_ref_count()
//               has already returned true.
////////////////////////////////////////////////////////////////////
INLINE TypedReferenceCount *Parameter::
get_typed_ref_count_value() const {
  return DCAST(TypedReferenceCount, get_typed_object());
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::get_typed_object
//       Access: Published
//  Description: Retrieves the pointer stored in the Parameter as a
//               TypedObject pointer.  It is only valid to call this
//               if is_pointer() has already returned true.
//               If this is the empty parameter, returns NULL without
//               raising an error.
////////////////////////////////////////////////////////////////////
INLINE TypedObject *Parameter::
get_typed_object() const {
  nassertr(do_get_type() == T_pointer, NULL);
  ReferenceCount *ptr = do_get_ptr();
  if (ptr == (ReferenceCount *)NULL) {
    // Empty parameter.
    return (TypedObject *)NULL;
  }
  TypedObject *obj = ptr->as_typed_object();
  nassertr(obj != NULL, NULL);
  return obj;
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::write_datagram
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Parameter::
write_datagram(BamWriter *manager, Datagram &destination) const {
  switch (do_get_type()) {
  case T_pointer:
    {
      TypedWritable *writable = NULL;
      DCAST_INTO_V(writable, get_typed_object());
      destination.add_uint8(0);
      manager->write_pointer(destination, writable);
      break;
    }
  case T_int:
    destination.add_uint8(1);
    destination.add_int32(_v._packed._int);
    break;

  case T_bool:
    destination.add_uint8(2);
    destination.add_bool(_v._packed._bool);
    break;

  case T_float:
    destination.add_uint8(3);
    destination.add_float32(_v._packed._float);
    break;

  default:
    destination.add_uint8(4);
    destination.add_float64(do_get_double());
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::read_datagram
//       Access: Published
//  Description: It is up to the caller to make sure to also call
//               complete_pointers.
////////////////////////////////////////////////////////////////////
INLINE void Parameter::
read_datagram(DatagramIterator &source, BamReader *manager) {
  memset(&_v, 0, sizeof(_v));

  unsigned char type = source.get_uint8();

  switch (type) {
  case 0:
    clear();
    manager->read_pointer(source);
    break;

  case 1:
    set_value(source.get_int32());
    break;

  case 2:
    set_value(source.get_bool());
    break;

  case 3:
    set_value(source.get_float32());
    break;

  case 4:
    set_value(source.get_float64());
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::complete_pointers
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int Parameter::
complete_pointers(TypedWritable **plist, BamReader *manager) {
  if (do_get_type() == T_pointer) {
    do_set_ptr(DCAST(TypedWritableReferenceCount, plist[0]));
    return 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::do_get_double
//       Access: Private
//  Description: Returns the double value without type checking.
//               Returns NaN if it is not a double.
////////////////////////////////////////////////////////////////////
INLINE double Parameter::
do_get_double() const {
  // We just have to flip the exponent space.
  Value v = _v;
  v._packed._type ^= T_MASK;
  return v._double;
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::do_set_double
//       Access: Private
//  Description: Sets the double value without regard for what is
//               currently being stored.
////////////////////////////////////////////////////////////////////
INLINE void Parameter::
do_set_double(double value) {
  _v._double = value;
  _v._packed._type ^= T_MASK;
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::do_get_ptr
//       Access: Private
//  Description: Returns the pointer value without type checking.
////////////////////////////////////////////////////////////////////
INLINE ReferenceCount *Parameter::
do_get_ptr() const {
#if NATIVE_WORDSIZE == 64
  return _v._ptr;
#else
  return _v._packed._ptr;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::do_set_ptr
//       Access: Private
//  Description: Changes the pointer value.  Warning: does not
//               decref the previous value, if any.
////////////////////////////////////////////////////////////////////
INLINE void Parameter::
do_set_ptr(ReferenceCount *ptr) {
  if (ptr != (ReferenceCount *)NULL) {
    ptr->ref();
  }

  _v._packed._type = T_pointer;

#if NATIVE_WORDSIZE == 64
  _v._ptr = ptr;
#else
  _v._packed._ptr = ptr;
#endif

  if (do_get_type() != 0) {
    cerr << "do_set_ptr " << ptr << " -> " << _v._ptr << "\n";
    cerr << _v._packed._type << "\n";
  }

  nassertv(do_get_type() == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: Parameter::do_get_type
//       Access: Private
//  Description: Returns the type tag.
////////////////////////////////////////////////////////////////////
INLINE Parameter::Type Parameter::
do_get_type() const {
  return (Type)(_v._packed._type & T_MASK);
}

INLINE void
swap(Parameter &one, Parameter &two) NOEXCEPT {
  one.swap(two);
}

INLINE ostream &
operator << (ostream &out, const Parameter &param) {
  param.output(out);
  return out;
}
