/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file hashVal.I
 * @author drose
 * @date 2000-11-14
 */

/**
 *
 */
INLINE HashVal::
HashVal() {
  _hv[0] = _hv[1] = _hv[2] = _hv[3] = 0;
}

/**
 *
 */
INLINE HashVal::
HashVal(const HashVal &copy) {
  _hv[0] = copy._hv[0];
  _hv[1] = copy._hv[1];
  _hv[2] = copy._hv[2];
  _hv[3] = copy._hv[3];
}

/**
 *
 */
INLINE void HashVal::
operator = (const HashVal &copy) {
  _hv[0] = copy._hv[0];
  _hv[1] = copy._hv[1];
  _hv[2] = copy._hv[2];
  _hv[3] = copy._hv[3];
}

/**
 *
 */
INLINE bool HashVal::
operator == (const HashVal &other) const {
  return (_hv[0] == other._hv[0] &&
          _hv[1] == other._hv[1] &&
          _hv[2] == other._hv[2] &&
          _hv[3] == other._hv[3]);
}

/**
 *
 */
INLINE bool HashVal::
operator != (const HashVal &other) const {
  return !operator == (other);
}

/**
 *
 */
INLINE bool HashVal::
operator < (const HashVal &other) const {
  return compare_to(other) < 0;
}

/**
 *
 */
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

/**
 * Generates a new HashVal representing the xor of this one and the other one.
 */
INLINE void HashVal::
merge_with(const HashVal &other) {
  _hv[0] ^= other._hv[0];
  _hv[1] ^= other._hv[1];
  _hv[2] ^= other._hv[2];
  _hv[3] ^= other._hv[3];
}

/**
 * Outputs the HashVal as four unsigned decimal integers.
 */
INLINE void HashVal::
output_dec(std::ostream &out) const {
  out << _hv[0] << " " << _hv[1] << " " << _hv[2] << " " << _hv[3];
}

/**
 * Inputs the HashVal as four unsigned decimal integers.
 */
INLINE void HashVal::
input_dec(std::istream &in) {
  in >> _hv[0] >> _hv[1] >> _hv[2] >> _hv[3];
}

/**
 *
 */
INLINE void HashVal::
output(std::ostream &out) const {
  output_hex(out);
}

/**
 *
 */
INLINE void HashVal::
write_datagram(Datagram &destination) const {
  destination.add_uint32(_hv[0]);
  destination.add_uint32(_hv[1]);
  destination.add_uint32(_hv[2]);
  destination.add_uint32(_hv[3]);
}

/**
 *
 */
INLINE void HashVal::
read_datagram(DatagramIterator &source) {
  _hv[0] = source.get_uint32();
  _hv[1] = source.get_uint32();
  _hv[2] = source.get_uint32();
  _hv[3] = source.get_uint32();
}

/**
 *
 */
INLINE void HashVal::
write_stream(StreamWriter &destination) const {
  destination.add_uint32(_hv[0]);
  destination.add_uint32(_hv[1]);
  destination.add_uint32(_hv[2]);
  destination.add_uint32(_hv[3]);
}

/**
 *
 */
INLINE void HashVal::
read_stream(StreamReader &source) {
  _hv[0] = source.get_uint32();
  _hv[1] = source.get_uint32();
  _hv[2] = source.get_uint32();
  _hv[3] = source.get_uint32();
}

#ifdef HAVE_OPENSSL
/**
 * Generates the hash value by hashing the indicated data.  This method is
 * only defined if we have the OpenSSL library (which provides md5
 * functionality) available.
 */
INLINE void HashVal::
hash_ramfile(const Ramfile &ramfile) {
  hash_buffer(ramfile._data.data(), ramfile._data.length());
}

/**
 * Generates the hash value by hashing the indicated data.  This method is
 * only defined if we have the OpenSSL library (which provides md5
 * functionality) available.
 */
INLINE void HashVal::
hash_string(const std::string &data) {
  hash_buffer(data.data(), data.length());
}

/**
 * Generates the hash value by hashing the indicated data.  This method is
 * only defined if we have the OpenSSL library (which provides md5
 * functionality) available.
 */
INLINE void HashVal::
hash_bytes(const vector_uchar &data) {
  hash_buffer((const char *)&data[0], data.size());
}
#endif  // HAVE_OPENSSL

/**
 * Converts a single nibble to a hex digit.
 */
INLINE char HashVal::
tohex(unsigned int nibble) {
  nibble &= 0xf;
  if (nibble < 10) {
    return nibble + '0';
  }
  return nibble - 10 + 'a';
}

/**
 * Converts a single hex digit to a numerical value.
 */
INLINE unsigned int HashVal::
fromhex(char digit) {
  if (isdigit(digit)) {
    return digit - '0';
  } else {
    return tolower(digit) - 'a' + 10;
  }
}


INLINE std::ostream &operator << (std::ostream &out, const HashVal &hv) {
  hv.output(out);
  return out;
}
