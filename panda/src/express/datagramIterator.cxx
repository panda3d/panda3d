// Filename: datagramIterator.cxx
// Created by:  jns (07Feb00)
// 

#include "datagramIterator.h"
#include "littleEndian.h"
#include "bigEndian.h"

#include <notify.h>

// Various ways to get data and increment the iterator...
// Cut-and-paste-orama

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_bool
//       Access: Public
//  Description: Extracts a boolean value.
////////////////////////////////////////////////////////////////////
bool DatagramIterator::
get_bool() {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0);

  PN_uint8 tempvar;
  LittleEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return (tempvar != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_int8
//       Access: Public
//  Description: Extracts a signed 8-bit integer.
////////////////////////////////////////////////////////////////////
PN_int8 DatagramIterator::
get_int8() {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0);

  PN_int8 tempvar;
  LittleEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_uint8
//       Access: Public
//  Description: Extracts an unsigned 8-bit integer.
////////////////////////////////////////////////////////////////////
PN_uint8 DatagramIterator::
get_uint8() {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0);

  PN_uint8 tempvar;
  LittleEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_int16
//       Access: Public
//  Description: Extracts a signed 16-bit integer.
////////////////////////////////////////////////////////////////////
PN_int16 DatagramIterator::
get_int16() {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0);

  PN_int16 tempvar;
  LittleEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_int32
//       Access: Public
//  Description: Extracts a signed 32-bit integer.
////////////////////////////////////////////////////////////////////
PN_int32 DatagramIterator::
get_int32() {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0);

  PN_int32 tempvar;
  LittleEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_int64
//       Access: Public
//  Description: Extracts a signed 64-bit integer.
////////////////////////////////////////////////////////////////////
PN_int64 DatagramIterator::
get_int64() {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0);

  PN_int64 tempvar;
  LittleEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_uint16
//       Access: Public
//  Description: Extracts an unsigned 16-bit integer.
////////////////////////////////////////////////////////////////////
PN_uint16 DatagramIterator::
get_uint16() {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0);

  PN_uint16 tempvar;
  LittleEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_uint32
//       Access: Public
//  Description: Extracts an unsigned 32-bit integer.
////////////////////////////////////////////////////////////////////
PN_uint32 DatagramIterator::
get_uint32() {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0);

  PN_uint32 tempvar;
  LittleEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_uint64
//       Access: Public
//  Description: Extracts an unsigned 64-bit integer.
////////////////////////////////////////////////////////////////////
PN_uint64 DatagramIterator::
get_uint64() {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0);

  PN_uint64 tempvar;
  LittleEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_float32
//       Access: Public
//  Description: Extracts a 32-bit single-precision floating-point
//               number.  Since this kind of float is not necessarily
//               portable across different architectures, special care
//               is required.
////////////////////////////////////////////////////////////////////
float DatagramIterator::
get_float32() {
  // For now, we assume the float format is portable across all
  // architectures we are concerned with.  If we come across one that
  // is different, we will have to convert.
  nassertr(sizeof(float) == 4, 0.0);
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0.0);

  float tempvar;
  LittleEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0.0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_float64
//       Access: Public
//  Description: Extracts a 64-bit floating-point number.
////////////////////////////////////////////////////////////////////
PN_float64 DatagramIterator::
get_float64() {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0.0);

  PN_float64 tempvar;
  LittleEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0.0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_be_int16
//       Access: Public
//  Description: Extracts a signed 16-bit big-endian integer.
////////////////////////////////////////////////////////////////////
PN_int16 DatagramIterator::
get_be_int16() {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0);

  PN_int16 tempvar;
  BigEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_be_int32
//       Access: Public
//  Description: Extracts a signed 32-bit big-endian integer.
////////////////////////////////////////////////////////////////////
PN_int32 DatagramIterator::
get_be_int32() {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0);

  PN_int32 tempvar;
  BigEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_be_int64
//       Access: Public
//  Description: Extracts a signed 64-bit big-endian integer.
////////////////////////////////////////////////////////////////////
PN_int64 DatagramIterator::
get_be_int64() {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0);

  PN_int64 tempvar;
  BigEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_be_uint16
//       Access: Public
//  Description: Extracts an unsigned 16-bit big-endian integer.
////////////////////////////////////////////////////////////////////
PN_uint16 DatagramIterator::
get_be_uint16() {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0);

  PN_uint16 tempvar;
  BigEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_be_uint32
//       Access: Public
//  Description: Extracts an unsigned 32-bit big-endian integer.
////////////////////////////////////////////////////////////////////
PN_uint32 DatagramIterator::
get_be_uint32() {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0);

  PN_uint32 tempvar;
  BigEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_be_uint64
//       Access: Public
//  Description: Extracts an unsigned 64-bit big-endian integer.
////////////////////////////////////////////////////////////////////
PN_uint64 DatagramIterator::
get_be_uint64() {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0);

  PN_uint64 tempvar;
  BigEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_be_float32
//       Access: Public
//  Description: Extracts a 32-bit big-endian single-precision
//               floating-point number.  Since this kind of float is
//               not necessarily portable across different
//               architectures, special care is required.
////////////////////////////////////////////////////////////////////
float DatagramIterator::
get_be_float32() {
  // For now, we assume the float format is portable across all
  // architectures we are concerned with.  If we come across one that
  // is different, we will have to convert.
  nassertr(sizeof(float) == 4, 0.0);
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0.0);

  float tempvar;
  BigEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0.0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_be_float64
//       Access: Public
//  Description: Extracts a 64-bit big-endian floating-point number.
////////////////////////////////////////////////////////////////////
PN_float64 DatagramIterator::
get_be_float64() {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index < _datagram->get_length(), 0.0);

  PN_float64 tempvar;
  BigEndian s = 
    _datagram->get_message().substr(_current_index, sizeof(tempvar));

  nassertr(s.length() == sizeof(tempvar), 0.0);
  memcpy((void *)&tempvar, (void *)s.data(), sizeof(tempvar));
  _current_index += sizeof(tempvar);

  return tempvar;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_string
//       Access: Public
//  Description: Extracts a variable-length string.
////////////////////////////////////////////////////////////////////
string DatagramIterator::
get_string() {
  // First, get the length of the string
  PN_uint16 s_len = get_uint16();

  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index + s_len <= _datagram->get_length(), "");

  string s = 
    _datagram->get_message().substr(_current_index, s_len);

  nassertr(s.length() == s_len, "");
  _current_index += s_len;

  return s;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_fixed_string
//       Access: Public
//  Description: Extracts a fixed-length string.  However, if a zero
//               byte occurs within the string, it marks the end of
//               the string.
////////////////////////////////////////////////////////////////////
string DatagramIterator::
get_fixed_string(size_t size) {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index + size <= _datagram->get_length(), "");

  string s = 
    _datagram->get_message().substr(_current_index, size);
  _current_index += size;

  size_t zero_byte = s.find('\0');
  return s.substr(0, zero_byte);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::skip_bytes
//       Access: Public
//  Description: Skips over the indicated number of bytes in the
//               datagram.
////////////////////////////////////////////////////////////////////
void DatagramIterator::
skip_bytes(size_t size) {
  nassertv((int)size >= 0);
  nassertv(_current_index + size <= _datagram->get_length());
  _current_index += size;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::extract_bytes
//       Access: Public
//  Description: Extracts the indicated number of bytes in the
//               datagram and returns them as a string.
////////////////////////////////////////////////////////////////////
string DatagramIterator::
extract_bytes(size_t size) {
  nassertr((int)size >= 0, "");
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index + size <= _datagram->get_length(), "");
  int start = _current_index;

  _current_index += size;
  return _datagram->get_message().substr(start, size);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_remaining_bytes
//       Access: Public
//  Description: Returns the remaining bytes in the datagram as a
//               string, but does not extract them from the iterator.
////////////////////////////////////////////////////////////////////
string DatagramIterator::
get_remaining_bytes() const {
  nassertr(_datagram != (const Datagram *)NULL &&
	   _current_index <= _datagram->get_length(), "");
  return _datagram->get_message().substr(_current_index);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_remaining_size
//       Access: Public
//  Description: Return the bytes left in the datagram.
////////////////////////////////////////////////////////////////////
int DatagramIterator::
get_remaining_size() const {
  return _datagram->get_length() - _current_index;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_datagram
//       Access: Public
//  Description: Return the datagram of this iterator.
////////////////////////////////////////////////////////////////////
const Datagram &DatagramIterator::
get_datagram() const {
  return *_datagram;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramIterator::get_current_index
//       Access: Public
//  Description: Returns the current position within the datagram of the
//               next piece of data to extract.
////////////////////////////////////////////////////////////////////
size_t DatagramIterator::
get_current_index() const {
  return _current_index;
}
