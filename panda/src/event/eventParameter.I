// Filename: eventParameter.I
// Created by:  drose (08Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////



template<class Type>
TypeHandle EventStoreValue<Type>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::Pointer constructor
//       Access: Public
//  Description: Defines an EventParameter that stores a pointer to
//               any kind of TypedReferenceCount object.  This is the
//               most general constructor.
////////////////////////////////////////////////////////////////////
INLINE EventParameter::
EventParameter(const TypedReferenceCount *ptr) : _ptr(ptr) { }


////////////////////////////////////////////////////////////////////
//     Function: EventParameter::Integer constructor
//       Access: Public
//  Description: Defines an EventParameter that stores an integer
//               value.
////////////////////////////////////////////////////////////////////
INLINE EventParameter::
EventParameter(int value) : _ptr(new EventStoreInt(value)) { }


////////////////////////////////////////////////////////////////////
//     Function: EventParameter::Double constructor
//       Access: Public
//  Description: Defines an EventParameter that stores a
//               floating-point value.
////////////////////////////////////////////////////////////////////
INLINE EventParameter::
EventParameter(double value) : _ptr(new EventStoreDouble(value)) { }


////////////////////////////////////////////////////////////////////
//     Function: EventParameter::String constructor
//       Access: Public
//  Description: Defines an EventParameter that stores a string value.
////////////////////////////////////////////////////////////////////
INLINE EventParameter::
EventParameter(const string &value) : _ptr(new EventStoreString(value)) { }


////////////////////////////////////////////////////////////////////
//     Function: EventParameter::LVecBase3f constructor
//       Access: Public
//  Description: Defines an EventParameter that stores a
//               three-component numeric value.
////////////////////////////////////////////////////////////////////
#if 0
INLINE EventParameter::
EventParameter(const LVecBase3f &value) : _ptr(new EventStoreVec3(value)) { }
#endif


////////////////////////////////////////////////////////////////////
//     Function: EventParameter::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EventParameter::
EventParameter(const EventParameter &other) : _ptr(other._ptr) { }


////////////////////////////////////////////////////////////////////
//     Function: EventParameter::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EventParameter &EventParameter::
operator = (const EventParameter &other) {
  _ptr = other._ptr;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::is_int
//       Access: Public
//  Description: Returns true if the EventParameter stores an integer
//               value, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EventParameter::
is_int() const {
  nassertr(_ptr != (TypedReferenceCount *)NULL, false);
  return _ptr->is_of_type(EventStoreInt::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::get_int_value
//       Access: Public
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
//       Access: Public
//  Description: Returns true if the EventParameter stores a double
//               floating-point value, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EventParameter::
is_double() const {
  nassertr(_ptr != (TypedReferenceCount *)NULL, false);
  return _ptr->is_of_type(EventStoreDouble::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::get_double_value
//       Access: Public
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
//       Access: Public
//  Description: Returns true if the EventParameter stores a string
//               value, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EventParameter::
is_string() const {
  nassertr(_ptr != (TypedReferenceCount *)NULL, false);
  return _ptr->is_of_type(EventStoreString::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::get_string_value
//       Access: Public
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
//     Function: EventParameter::is_vec3
//       Access: Public
//  Description: Returns true if the EventParameter stores a
//               three-component vector value, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EventParameter::
is_vec3() const {
  nassertr(_ptr != (TypedReferenceCount *)NULL, false);
  return false;
//  return _ptr->is_of_type(EventStoreVec3::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::get_vec3_value
//       Access: Public
//  Description: Retrieves the value stored in the EventParameter.  It
//               is only valid to call this if is_vec3() has already
//               returned true.
////////////////////////////////////////////////////////////////////
#if 0
INLINE LVecBase3f EventParameter::
get_vec3_value() const {
  nassertr(is_vec3(), LVecBase3f(0.0, 0.0, 0.0));
  return ((const EventStoreVec3 *)_ptr.p())->get_value();
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::get_ptr
//       Access: Public
//  Description: Retrieves a pointer to the actual value stored in the
//               parameter.  The TypeHandle of this pointer may be
//               examined to determine the actual type of parameter it
//               contains.  This is the only way to retrieve the value
//               when it is not one of the above predefined types.
////////////////////////////////////////////////////////////////////
INLINE const TypedReferenceCount *EventParameter::
get_ptr() const {
  return _ptr;
}


////////////////////////////////////////////////////////////////////
//     Function: EventStoreValue::set_value
//       Access: Public
//  Description: Changes the value stored in the parameter.  It is
//               dangerous to do this for a parameter already added to
//               an event, since the parameters may be shared.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE void EventStoreValue<Type>::
set_value(const Type &value) {
  _value = value;
}


////////////////////////////////////////////////////////////////////
//     Function: EventStoreValue::get_value
//       Access: Public
//  Description: Retrieves the value stored in the parameter.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE Type EventStoreValue<Type>::
get_value() const {
  return _value;
}
