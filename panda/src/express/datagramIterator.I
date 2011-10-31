// Filename: datagramIterator.I
// Created by:  drose (08May01)
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
//     Function: DatagramIterator::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DatagramIterator::
DatagramIterator() :
    _datagram((Datagram *)NULL),
    _current_index(0) {
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DatagramIterator::
DatagramIterator(const Datagram &datagram, size_t offset) :
    _datagram(&datagram),
    _current_index(offset) {
  nassertv(_current_index <= _datagram->get_length());
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DatagramIterator::
DatagramIterator(const DatagramIterator &copy) :
    _datagram(copy._datagram),
  _current_index(copy._current_index) {
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DatagramIterator::
operator = (const DatagramIterator &copy) {
  _datagram = copy._datagram;
  _current_index = copy._current_index;
}
////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::assign
//       Access: Public
//  Description: direct Assignment to a Datagram
////////////////////////////////////////////////////////////////////
INLINE void DatagramIterator::assign(Datagram &datagram, size_t offset)
{
    _datagram =&datagram;
    _current_index = offset;     
}
////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DatagramIterator::
~DatagramIterator() {
}

// Various ways to get data and increment the iterator...
// Cut-and-paste-orama

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_bool
//       Access: Public
//  Description: Extracts a boolean value.
////////////////////////////////////////////////////////////////////
INLINE bool DatagramIterator::
get_bool() {
  return get_uint8() != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_int8
//       Access: Public
//  Description: Extracts a signed 8-bit integer.
////////////////////////////////////////////////////////////////////
INLINE PN_int8 DatagramIterator::
get_int8() {
  nassertr(_datagram != (const Datagram *)NULL, 0);
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index < _datagram->get_length(), 0);
  // Get the Data:
  const char *ptr = (const char *)_datagram->get_data();
  PN_int8 tempvar = (PN_int8)ptr[_current_index];
  ++_current_index;

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_uint8
//       Access: Public
//  Description: Extracts an unsigned 8-bit integer.
////////////////////////////////////////////////////////////////////
INLINE PN_uint8 DatagramIterator::
get_uint8() {
  nassertr(_datagram != (const Datagram *)NULL, 0);
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index < _datagram->get_length(), 0);
  // Get the Data:
  const char *ptr = (const char *)_datagram->get_data();
  PN_uint8 tempvar = (PN_uint8)ptr[_current_index];
  ++_current_index;

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_int16
//       Access: Public
//  Description: Extracts a signed 16-bit integer.
////////////////////////////////////////////////////////////////////
INLINE PN_int16 DatagramIterator::
get_int16() {
  nassertr(_datagram != (const Datagram *)NULL, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  PN_int16 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  LittleEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_int32
//       Access: Public
//  Description: Extracts a signed 32-bit integer.
////////////////////////////////////////////////////////////////////
INLINE PN_int32 DatagramIterator::
get_int32() {
  nassertr(_datagram != (const Datagram *)NULL, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  PN_int32 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  LittleEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_int64
//       Access: Public
//  Description: Extracts a signed 64-bit integer.
////////////////////////////////////////////////////////////////////
INLINE PN_int64 DatagramIterator::
get_int64() {
  nassertr(_datagram != (const Datagram *)NULL, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  PN_int64 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  LittleEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_uint16
//       Access: Public
//  Description: Extracts an unsigned 16-bit integer.
////////////////////////////////////////////////////////////////////
INLINE PN_uint16 DatagramIterator::
get_uint16() {
  nassertr(_datagram != (const Datagram *)NULL, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  PN_uint16 tempvar;  
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  LittleEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_uint32
//       Access: Public
//  Description: Extracts an unsigned 32-bit integer.
////////////////////////////////////////////////////////////////////
INLINE PN_uint32 DatagramIterator::
get_uint32() {
  nassertr(_datagram != (const Datagram *)NULL, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  PN_uint32 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  LittleEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_uint64
//       Access: Public
//  Description: Extracts an unsigned 64-bit integer.
////////////////////////////////////////////////////////////////////
INLINE PN_uint64 DatagramIterator::
get_uint64() {
  nassertr(_datagram != (const Datagram *)NULL, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  PN_uint64 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  LittleEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_float32
//       Access: Public
//  Description: Extracts a 32-bit single-precision floating-point
//               number.
////////////////////////////////////////////////////////////////////
INLINE PN_float32 DatagramIterator::
get_float32() {
  nassertr(_datagram != (const Datagram *)NULL, 0.0);
  nassertr(_current_index < _datagram->get_length(), 0.0);

  PN_float32 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0.0);
  // Get the Data:
  LittleEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_float64
//       Access: Public
//  Description: Extracts a 64-bit floating-point number.
////////////////////////////////////////////////////////////////////
INLINE PN_float64 DatagramIterator::
get_float64() {
  nassertr(_datagram != (const Datagram *)NULL, 0.0);
  nassertr(_current_index < _datagram->get_length(), 0.0);

  PN_float64 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0.0);
  // Get the Data:
  LittleEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}


////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_stdfloat
//       Access: Public
//  Description: Extracts either a 32-bit or a 64-bit floating-point
//               number, according to Datagram::set_stdfloat_double().
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DatagramIterator::
get_stdfloat() {
  nassertr(_datagram != (const Datagram *)NULL, 0.0);
  if (_datagram->get_stdfloat_double()) {
    return (PN_stdfloat)get_float64();
  } else {
    return (PN_stdfloat)get_float32();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_be_int16
//       Access: Public
//  Description: Extracts a signed 16-bit big-endian integer.
////////////////////////////////////////////////////////////////////
INLINE PN_int16 DatagramIterator::
get_be_int16() {
  nassertr(_datagram != (const Datagram *)NULL, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  PN_int16 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  BigEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_be_int32
//       Access: Public
//  Description: Extracts a signed 32-bit big-endian integer.
////////////////////////////////////////////////////////////////////
INLINE PN_int32 DatagramIterator::
get_be_int32() {
  nassertr(_datagram != (const Datagram *)NULL, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  PN_int32 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  BigEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_be_int64
//       Access: Public
//  Description: Extracts a signed 64-bit big-endian integer.
////////////////////////////////////////////////////////////////////
INLINE PN_int64 DatagramIterator::
get_be_int64() {
  nassertr(_datagram != (const Datagram *)NULL, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  PN_int64 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  BigEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_be_uint16
//       Access: Public
//  Description: Extracts an unsigned 16-bit big-endian integer.
////////////////////////////////////////////////////////////////////
INLINE PN_uint16 DatagramIterator::
get_be_uint16() {
  nassertr(_datagram != (const Datagram *)NULL, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  PN_uint16 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  BigEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_be_uint32
//       Access: Public
//  Description: Extracts an unsigned 32-bit big-endian integer.
////////////////////////////////////////////////////////////////////
INLINE PN_uint32 DatagramIterator::
get_be_uint32() {
  nassertr(_datagram != (const Datagram *)NULL, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  PN_uint32 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  BigEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_be_uint64
//       Access: Public
//  Description: Extracts an unsigned 64-bit big-endian integer.
////////////////////////////////////////////////////////////////////
INLINE PN_uint64 DatagramIterator::
get_be_uint64() {
  nassertr(_datagram != (const Datagram *)NULL, 0);
  nassertr(_current_index < _datagram->get_length(), 0);

  PN_uint64 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  BigEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_be_float32
//       Access: Public
//  Description: Extracts a 32-bit big-endian single-precision
//               floating-point number.
////////////////////////////////////////////////////////////////////
INLINE PN_float32 DatagramIterator::
get_be_float32() {
  nassertr(_datagram != (const Datagram *)NULL, 0.0);
  nassertr(_current_index < _datagram->get_length(), 0.0);

  PN_float32 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0);
  // Get the Data:
  BigEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_be_float64
//       Access: Public
//  Description: Extracts a 64-bit big-endian floating-point number.
////////////////////////////////////////////////////////////////////
INLINE PN_float64 DatagramIterator::
get_be_float64() {
  nassertr(_datagram != (const Datagram *)NULL, 0.0);
  nassertr(_current_index < _datagram->get_length(), 0.0);

  PN_float64 tempvar;
  // Avoid reading junk data off the end of the datagram:
  nassertr(_current_index + sizeof(tempvar) <= _datagram->get_length(), 0.0);
  // Get the Data:
  BigEndian s(_datagram->get_data(), _current_index, sizeof(tempvar));
  s.store_value(&tempvar, sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::skip_bytes
//       Access: Public
//  Description: Skips over the indicated number of bytes in the
//               datagram.
////////////////////////////////////////////////////////////////////
INLINE void DatagramIterator::
skip_bytes(size_t size) {
  nassertv(_datagram != (const Datagram *)NULL);
  nassertv((int)size >= 0);
#ifndef NDEBUG
  if (_current_index + size > _datagram->get_length()) {
     nout << "datagram overflow: current_index = " << _current_index
          << " size = " << size << " length = " << _datagram->get_length() << "\n";
    _datagram->dump_hex(nout);
  }
#endif
  nassertv(_current_index + size <= _datagram->get_length());
  _current_index += size;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_remaining_bytes
//       Access: Public
//  Description: Returns the remaining bytes in the datagram as a
//               string, but does not extract them from the iterator.
////////////////////////////////////////////////////////////////////
INLINE string DatagramIterator::
get_remaining_bytes() const {
  nassertr(_datagram != (const Datagram *)NULL, "");
  nassertr(_current_index <= _datagram->get_length(), "");

  const char *ptr = (const char *)_datagram->get_data();
  int remaining_size = _datagram->get_length() - _current_index;
  return string(ptr + _current_index, remaining_size);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_remaining_size
//       Access: Public
//  Description: Return the bytes left in the datagram.
////////////////////////////////////////////////////////////////////
INLINE int DatagramIterator::
get_remaining_size() const {
  return _datagram->get_length() - _current_index;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_datagram
//       Access: Public
//  Description: Return the datagram of this iterator.
////////////////////////////////////////////////////////////////////
INLINE const Datagram &DatagramIterator::
get_datagram() const {
  return *_datagram;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_current_index
//       Access: Public
//  Description: Returns the current position within the datagram of the
//               next piece of data to extract.
////////////////////////////////////////////////////////////////////
INLINE size_t DatagramIterator::
get_current_index() const {
  return _current_index;
}

INLINE void
generic_read_datagram(bool &result, DatagramIterator &source) {
  result = source.get_bool();
}

INLINE void
generic_read_datagram(int &result, DatagramIterator &source) {
  result = source.get_int32();
}

INLINE void
generic_read_datagram(float &result, DatagramIterator &source) {
  result = source.get_float32();
}

INLINE void
generic_read_datagram(double &result, DatagramIterator &source) {
  result = source.get_float64();
}

INLINE void
generic_read_datagram(string &result, DatagramIterator &source) {
  result = source.get_string();
}

INLINE void
generic_read_datagram(wstring &result, DatagramIterator &source) {
  result = source.get_wstring();
}


