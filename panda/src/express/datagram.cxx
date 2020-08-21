/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagram.cxx
 * @author drose
 * @date 2000-06-06
 */

#include "datagram.h"

#include "pnotify.h"

// for sprintf().
#include <stdio.h>

TypeHandle Datagram::_type_handle;

/**
 *
 */
Datagram::
~Datagram() {
}

/**
 * Resets the datagram to empty, in preparation for building up a new
 * datagram.
 */
void Datagram::
clear() {
  _data.clear();
}

/**
 * Writes a representation of the entire datagram contents, as a sequence of
 * hex (and ASCII) values.
 */
void Datagram::
dump_hex(std::ostream &out, unsigned int indent) const {
  const char *message = (const char *)get_data();
  size_t num_bytes = get_length();
  for (size_t line = 0; line < num_bytes; line += 16) {
    char hex[12];
    sprintf(hex, "%04x ", ((unsigned int )line));
    for (unsigned int ind = 0; ind < indent; ind++) {
      out << " ";
    }
    out << hex;

    size_t p;
    for (p = line; p < line + 16; p++) {
      if (p < num_bytes) {
        char hex[12];
        sprintf(hex, " %02x", ((unsigned int)message[p]) & 0xff);
        out << hex;
      } else {
        out << "   ";
      }
    }
    out << "  ";
    for (p = line; p < line + 16 && p < num_bytes; p++) {
      // must cast to (unsigned char) to avoid conversion to large negative
      // integers outside of 0xFF range
      if (isgraph((unsigned char)message[p]) || message[p] == ' ') {
        out << (char)message[p];
      } else {
        out << ".";
      }
    }
    out << "\n";
  }
}

/**
 * Adds a variable-length wstring to the datagram.
 */
void Datagram::
add_wstring(const std::wstring &str) {
  // By convention, wstrings are marked with 32-bit lengths.
  add_uint32((uint32_t)str.length());

  // Now append each character in the string.  We store each code little-
  // endian, for no real good reason.
  std::wstring::const_iterator ci;
  for (ci = str.begin(); ci != str.end(); ++ci) {
    add_uint16((uint16_t)*ci);
  }
}

/**
 * Adds the indicated number of zero bytes to the datagram.
 */
void Datagram::
pad_bytes(size_t size) {
  nassertv((int)size >= 0);

  if (_data == nullptr) {
    // Create a new array.
    _data = PTA_uchar::empty_array(0);

  } else if (_data.get_ref_count() != 1) {
    // Copy on write.
    PTA_uchar new_data = PTA_uchar::empty_array(0);
    new_data.v() = _data.v();
    _data = new_data;
  }

  // Now append the data.

  // It is very important that we *don't* do this reserve() operation.  See
  // the further comments in append_data(), below.  _data.reserve(_data.size()
  // + size);

  while (size > 0) {
    _data.push_back('\0');
    --size;
  }
}

/**
 * Appends some more raw data to the end of the datagram.
 */
void Datagram::
append_data(const void *data, size_t size) {
  nassertv((int)size >= 0);

  if (_data == nullptr) {
    // Create a new array.
    _data = PTA_uchar::empty_array(0);

  } else if (_data.get_ref_count() != 1) {
    // Copy on write.
    PTA_uchar new_data = PTA_uchar::empty_array(0);
    new_data.v() = _data.v();
    _data = new_data;
  }

  // Now append the data.

  // It is very important that we *don't* do this reserve() operation.  This
  // actually slows it down on Windows, which takes the reserve() request as a
  // fixed size the array should be set to (!) instead of as a minimum size to
  // guarantee.  This forces the array to reallocate itself with *every* call
  // to append_data!  _data.reserve(_data.size() + size);

  _data.v().insert(_data.v().end(), (const unsigned char *)data,
                   (const unsigned char *)data + size);
}

/**
 * Replaces the datagram's data with the indicated block.
 */
void Datagram::
assign(const void *data, size_t size) {
  nassertv((int)size >= 0);

  _data = PTA_uchar::empty_array(0);
  _data.v().insert(_data.v().end(), (const unsigned char *)data,
                   (const unsigned char *)data + size);
}

/**
 * Write a string representation of this instance to <out>.
 */
void Datagram::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<""<<"Datagram";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void Datagram::
write(std::ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent);
  out<<""<<"Datagram:\n";
  dump_hex(out, indent);
  #endif //] NDEBUG
}
