/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paramValue.I
 * @author drose
 * @date 1999-02-08
 */

template<class Type>
TypeHandle ParamValue<Type>::_type_handle;

/**
 *
 */
INLINE ParamValueBase::
ParamValueBase() {
}

/**
 * Returns the type of the underlying value.
 */
INLINE TypeHandle ParamValueBase::
get_value_type() const {
  return TypeHandle::none();
}

/**
 *
 */
INLINE ParamTypedRefCount::
ParamTypedRefCount(const TypedReferenceCount *value) :
  _value((TypedReferenceCount *)value)
{
}

/**
 * Retrieves the type of the value stored in the parameter.
 */
INLINE TypeHandle ParamTypedRefCount::
get_value_type() const {
  if (_value == nullptr) {
    return TypeHandle::none();
  } else {
    return _value->get_type();
  }
}

/**
 * Retrieves the value stored in the parameter.
 */
INLINE TypedReferenceCount *ParamTypedRefCount::
get_value() const {
  return _value;
}

/**
 *
 */
template<class Type>
INLINE ParamValue<Type>::
ParamValue() {}

/**
 *
 */
template<class Type>
INLINE ParamValue<Type>::
ParamValue(const Type &value) :
  _value(value)
{
}

/**
 *
 */
template<class Type>
INLINE ParamValue<Type>::
~ParamValue() {
}

/**
 * Retrieves the type of the value stored in the parameter.
 */
template<class Type>
INLINE TypeHandle ParamValue<Type>::
get_value_type() const {
  return get_type_handle(Type);
}

/**
 * Changes the value stored in the parameter.
 */
template<class Type>
INLINE void ParamValue<Type>::
set_value(const Type &type) {
  _value = type;
  mark_bam_modified();
}

/**
 * Retrieves the value stored in the parameter.
 */
template<class Type>
INLINE const Type &ParamValue<Type>::
get_value() const {
  return _value;
}

/**
 *
 */
template<class Type>
INLINE void ParamValue<Type>::
output(std::ostream &out) const {
  out << _value;
}

/**
 * Tells the BamReader how to create objects of type ParamValue.
 */
template<class Type>
INLINE void ParamValue<Type>::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
template<class Type>
INLINE void ParamValue<Type>::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);
  generic_write_datagram(dg, _value);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ParamValue is encountered in the Bam file.  It should create the
 * ParamValue and extract its information from the file.
 */
template<class Type>
INLINE TypedWritable *ParamValue<Type>::
make_from_bam(const FactoryParams &params) {
  ParamValue<Type> *esv = new ParamValue<Type>;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  esv->fillin(scan, manager);

  return esv;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ParamValue.
 */
template<class Type>
INLINE void ParamValue<Type>::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
  generic_read_datagram(_value, scan);
}
