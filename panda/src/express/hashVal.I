// Filename: hashVal.I
// Created by:  drose (14Nov00)
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
//     Function: HashVal::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE HashVal::
HashVal() {
  _hv[0] = _hv[1] = _hv[2] = _hv[3] = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE HashVal::
HashVal(const HashVal &copy) {
  _hv[0] = copy._hv[0];
  _hv[1] = copy._hv[1];
  _hv[2] = copy._hv[2];
  _hv[3] = copy._hv[3];
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void HashVal::
operator = (const HashVal &copy) {
  _hv[0] = copy._hv[0];
  _hv[1] = copy._hv[1];
  _hv[2] = copy._hv[2];
  _hv[3] = copy._hv[3];
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::operator ==
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool HashVal::
operator == (const HashVal &other) const {
  return (_hv[0] == other._hv[0] &&
          _hv[1] == other._hv[1] &&
          _hv[2] == other._hv[2] &&
          _hv[3] == other._hv[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::operator !=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool HashVal::
operator != (const HashVal &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::operator <
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool HashVal::
operator < (const HashVal &other) const {
  return compare_to(other) < 0;
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::compare_to
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int HashVal::
compare_to(const HashVal &other) const {
  if (_hv[0] != other._hv[0]) {
    return (int)_hv[0] - (int)other._hv[0];
  }
  if (_hv[1] != other._hv[1]) {
    return (int)_hv[1] - (int)other._hv[1];
  }
  if (_hv[2] != other._hv[2]) {
    return (int)_hv[2] - (int)other._hv[2];
  }
  return (int)_hv[3] - (int)other._hv[3];
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::merge_with
//       Access: Published
//  Description: Generates a new HashVal representing the xor of this
//               one and the other one.
////////////////////////////////////////////////////////////////////
INLINE void HashVal::
merge_with(const HashVal &other) {
  _hv[0] ^= other._hv[0];
  _hv[1] ^= other._hv[1];
  _hv[2] ^= other._hv[2];
  _hv[3] ^= other._hv[3];
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::output_dec
//       Access: Published
//  Description: Outputs the HashVal as four unsigned decimal
//               integers.
////////////////////////////////////////////////////////////////////
INLINE void HashVal::
output_dec(ostream &out) const {
  out << _hv[0] << " " << _hv[1] << " " << _hv[2] << " " << _hv[3];
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::input
//       Access: Published
//  Description: Inputs the HashVal as four unsigned decimal integers.
////////////////////////////////////////////////////////////////////
INLINE void HashVal::
input_dec(istream &in) {
  in >> _hv[0] >> _hv[1] >> _hv[2] >> _hv[3];
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void HashVal::
output(ostream &out) const {
  output_hex(out);
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::write_datagram
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void HashVal::
write_datagram(Datagram &destination) const {
  destination.add_uint32(_hv[0]);
  destination.add_uint32(_hv[1]);
  destination.add_uint32(_hv[2]);
  destination.add_uint32(_hv[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::read_datagram
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void HashVal::
read_datagram(DatagramIterator &source) {
  _hv[0] = source.get_uint32();
  _hv[1] = source.get_uint32();
  _hv[2] = source.get_uint32();
  _hv[3] = source.get_uint32();
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::write_stream
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void HashVal::
write_stream(StreamWriter &destination) const {
  destination.add_uint32(_hv[0]);
  destination.add_uint32(_hv[1]);
  destination.add_uint32(_hv[2]);
  destination.add_uint32(_hv[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::read_stream
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void HashVal::
read_stream(StreamReader &source) {
  _hv[0] = source.get_uint32();
  _hv[1] = source.get_uint32();
  _hv[2] = source.get_uint32();
  _hv[3] = source.get_uint32();
}

#ifdef HAVE_OPENSSL
////////////////////////////////////////////////////////////////////
//     Function: HashVal::hash_ramfile
//       Access: Published
//  Description: Generates the hash value by hashing the indicated
//               data.  This method is only defined if we have the
//               OpenSSL library (which provides md5 functionality)
//               available.
////////////////////////////////////////////////////////////////////
INLINE void HashVal::
hash_ramfile(const Ramfile &ramfile) {
  hash_buffer(ramfile._data.data(), ramfile._data.length());
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::hash_string
//       Access: Published
//  Description: Generates the hash value by hashing the indicated
//               data.  This method is only defined if we have the
//               OpenSSL library (which provides md5 functionality)
//               available.
////////////////////////////////////////////////////////////////////
INLINE void HashVal::
hash_string(const string &data) {
  hash_buffer(data.data(), data.length());
}
#endif  // HAVE_OPENSSL

////////////////////////////////////////////////////////////////////
//     Function: HashVal::tohex
//       Access: Private, Static
//  Description: Converts a single nibble to a hex digit.
////////////////////////////////////////////////////////////////////
INLINE char HashVal::
tohex(unsigned int nibble) {
  nibble &= 0xf;
  if (nibble < 10) {
    return nibble + '0';
  }
  return nibble - 10 + 'a';
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::fromhex
//       Access: Private, Static
//  Description: Converts a single hex digit to a numerical value.
////////////////////////////////////////////////////////////////////
INLINE unsigned int HashVal::
fromhex(char digit) {
  if (isdigit(digit)) {
    return digit - '0';
  } else {
    return tolower(digit) - 'a' + 10;
  }
}


INLINE ostream &operator << (ostream &out, const HashVal &hv) {
  hv.output(out);
  return out;
}
