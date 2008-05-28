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



template<class Type>
TypeHandle EventStoreValue<Type>::_type_handle;

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


////////////////////////////////////////////////////////////////////
//     Function: EventStoreValueBase::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EventStoreValueBase::
EventStoreValueBase() {
}

////////////////////////////////////////////////////////////////////
//     Function: EventStoreTypedRefCount::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EventStoreTypedRefCount::
EventStoreTypedRefCount(const TypedReferenceCount *value) : 
  _value((TypedReferenceCount *)value) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: EventStoreTypedRefCount::set_value
//       Access: Published
//  Description: Changes the value stored in the parameter.  It is
//               dangerous to do this for a parameter already added to
//               an event, since the parameters may be shared.
////////////////////////////////////////////////////////////////////
INLINE void EventStoreTypedRefCount::
set_value(const TypedReferenceCount *value) {
  _value = (TypedReferenceCount *)value;
}


////////////////////////////////////////////////////////////////////
//     Function: EventStoreTypedRefCount::get_value
//       Access: Published
//  Description: Retrieves the value stored in the parameter.
////////////////////////////////////////////////////////////////////
INLINE TypedReferenceCount *EventStoreTypedRefCount::
get_value() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: EventStoreValue::Default Constructor
//       Access: Private
//  Description: This constructor is only for make_from_bam().
////////////////////////////////////////////////////////////////////
template<class Type>
EventStoreValue<Type>::
EventStoreValue() {
}

////////////////////////////////////////////////////////////////////
//     Function: EventStoreValue::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Type>
EventStoreValue<Type>::
EventStoreValue(const Type &value) : 
  _value(value) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: EventStoreValue::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Type>
EventStoreValue<Type>::
~EventStoreValue() {
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
INLINE const Type &EventStoreValue<Type>::
get_value() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: EventStoreValue::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Type>
void EventStoreValue<Type>::
output(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: EventStoreValue::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               Lens.
////////////////////////////////////////////////////////////////////
template<class Type>
void EventStoreValue<Type>::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: EventStoreValue::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
template<class Type>
void EventStoreValue<Type>::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);
  generic_write_datagram(dg, _value);
}

////////////////////////////////////////////////////////////////////
//     Function: EventStoreValue::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type Lens is encountered
//               in the Bam file.  It should create the Lens
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
template<class Type>
TypedWritable *EventStoreValue<Type>::
make_from_bam(const FactoryParams &params) {
  EventStoreValue<Type> *esv = new EventStoreValue<Type>;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  esv->fillin(scan, manager);

  return esv;
}

////////////////////////////////////////////////////////////////////
//     Function: EventStoreValue::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new Lens.
////////////////////////////////////////////////////////////////////
template<class Type>
void EventStoreValue<Type>::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
  generic_read_datagram(_value, scan);
}
