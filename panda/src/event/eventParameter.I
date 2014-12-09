// Filename: eventParameter.I
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
//     Function: EventParameter::Default constructor
//       Access: Published
//  Description: Defines an EventParameter that stores nothing: the
//               "empty" parameter.
////////////////////////////////////////////////////////////////////
INLINE EventParameter::
EventParameter() {
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::Pointer constructor
//       Access: Published
//  Description: Defines an EventParameter that stores a pointer to
//               any kind of TypedWritableReferenceCount object.  This
//               is the most general constructor.
//
//               This accepts a const pointer, even though it stores
//               (and eventually returns) a non-const pointer.  This
//               is just the simplest way to allow both const and
//               non-const pointers to be stored, but it does lose the
//               constness.  Be careful.
////////////////////////////////////////////////////////////////////
INLINE EventParameter::
EventParameter(const TypedWritableReferenceCount *ptr) : _ptr((TypedWritableReferenceCount *)ptr) { }

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::Pointer constructor
//       Access: Published
//  Description: Defines an EventParameter that stores a pointer to
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
INLINE EventParameter::
EventParameter(const TypedReferenceCount *ptr) : _ptr(new EventStoreTypedRefCount((TypedReferenceCount *)ptr)) { }


////////////////////////////////////////////////////////////////////
//     Function: EventParameter::Integer constructor
//       Access: Published
//  Description: Defines an EventParameter that stores an integer
//               value.
////////////////////////////////////////////////////////////////////
INLINE EventParameter::
EventParameter(int value) : _ptr(new EventStoreInt(value)) { }


////////////////////////////////////////////////////////////////////
//     Function: EventParameter::Double constructor
//       Access: Published
//  Description: Defines an EventParameter that stores a
//               floating-point value.
////////////////////////////////////////////////////////////////////
INLINE EventParameter::
EventParameter(double value) : _ptr(new EventStoreDouble(value)) { }


////////////////////////////////////////////////////////////////////
//     Function: EventParameter::String constructor
//       Access: Published
//  Description: Defines an EventParameter that stores a string value.
////////////////////////////////////////////////////////////////////
INLINE EventParameter::
EventParameter(const string &value) : _ptr(new EventStoreString(value)) { }

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::Wstring constructor
//       Access: Published
//  Description: Defines an EventParameter that stores a wstring value.
////////////////////////////////////////////////////////////////////
INLINE EventParameter::
EventParameter(const wstring &value) : _ptr(new EventStoreWstring(value)) { }


////////////////////////////////////////////////////////////////////
//     Function: EventParameter::Copy constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EventParameter::
EventParameter(const EventParameter &other) : _ptr(other._ptr) { }


////////////////////////////////////////////////////////////////////
//     Function: EventParameter::Copy assignment operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EventParameter &EventParameter::
operator = (const EventParameter &other) {
  _ptr = other._ptr;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EventParameter::
~EventParameter() {
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::is_empty
//       Access: Published
//  Description: Returns true if the EventParameter is the empty
//               parameter, storing nothing, or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EventParameter::
is_empty() const {
  return (_ptr == (TypedWritableReferenceCount *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::is_int
//       Access: Published
//  Description: Returns true if the EventParameter stores an integer
//               value, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EventParameter::
is_int() const {
  if (is_empty()) {
    return false;
  }
  return _ptr->is_of_type(EventStoreInt::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::get_int_value
//       Access: Published
//  Description: Retrieves the value stored in the EventParameter.  It
//               is only valid to call this if is_int() has already
//               returned true.
////////////////////////////////////////////////////////////////////
INLINE int EventParameter::
get_int_value() const {
  nassertr(is_int(), 0);
  // We can't use DCAST, because EventStoreValue::init_type() breaks
  // convention and takes a parameter.  But the above assertion should
  // protect us.
  return ((const EventStoreInt *)_ptr.p())->get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::is_double
//       Access: Published
//  Description: Returns true if the EventParameter stores a double
//               floating-point value, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EventParameter::
is_double() const {
  if (is_empty()) {
    return false;
  }
  return _ptr->is_of_type(EventStoreDouble::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::get_double_value
//       Access: Published
//  Description: Retrieves the value stored in the EventParameter.  It
//               is only valid to call this if is_double() has already
//               returned true.
////////////////////////////////////////////////////////////////////
INLINE double EventParameter::
get_double_value() const {
  nassertr(is_double(), 0.0);
  return ((const EventStoreDouble *)_ptr.p())->get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::is_string
//       Access: Published
//  Description: Returns true if the EventParameter stores a string
//               value, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EventParameter::
is_string() const {
  if (is_empty()) {
    return false;
  }
  return _ptr->is_of_type(EventStoreString::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::get_string_value
//       Access: Published
//  Description: Retrieves the value stored in the EventParameter.  It
//               is only valid to call this if is_string() has already
//               returned true.
////////////////////////////////////////////////////////////////////
INLINE string EventParameter::
get_string_value() const {
  nassertr(is_string(), "");
  return ((const EventStoreString *)_ptr.p())->get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::is_wstring
//       Access: Published
//  Description: Returns true if the EventParameter stores a wstring
//               value, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EventParameter::
is_wstring() const {
  if (is_empty()) {
    return false;
  }
  return _ptr->is_of_type(EventStoreWstring::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::get_wstring_value
//       Access: Published
//  Description: Retrieves the value stored in the EventParameter.  It
//               is only valid to call this if is_wstring() has already
//               returned true.
////////////////////////////////////////////////////////////////////
INLINE wstring EventParameter::
get_wstring_value() const {
  nassertr(is_wstring(), wstring());
  return ((const EventStoreWstring *)_ptr.p())->get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::is_typed_ref_count
//       Access: Published
//  Description: Returns true if the EventParameter stores a
//               TypedReferenceCount pointer, false otherwise.  Note
//               that a TypedReferenceCount is not exactly the same
//               kind of pointer as a TypedWritableReferenceCount,
//               hence the need for this separate call.
////////////////////////////////////////////////////////////////////
INLINE bool EventParameter::
is_typed_ref_count() const {
  if (is_empty()) {
    return false;
  }
  return _ptr->is_of_type(EventStoreTypedRefCount::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::get_typed_ref_count_value
//       Access: Published
//  Description: Retrieves the value stored in the EventParameter.  It
//               is only valid to call this if is_typed_ref_count()
//               has already returned true.
////////////////////////////////////////////////////////////////////
INLINE TypedReferenceCount *EventParameter::
get_typed_ref_count_value() const {
  nassertr(is_typed_ref_count(), NULL);
  return ((const EventStoreTypedRefCount *)_ptr.p())->get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::get_ptr
//       Access: Published
//  Description: Retrieves a pointer to the actual value stored in the
//               parameter.  The TypeHandle of this pointer may be
//               examined to determine the actual type of parameter it
//               contains.  This is the only way to retrieve the value
//               when it is not one of the above predefined types.
////////////////////////////////////////////////////////////////////
INLINE TypedWritableReferenceCount *EventParameter::
get_ptr() const {
  return _ptr;
}

INLINE ostream &
operator << (ostream &out, const EventParameter &param) {
  param.output(out);
  return out;
}
