/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoVertexMap.cxx
 * @author drose
 * @date 2001-04-24
 */

#include "lwoVertexMap.h"
#include "lwoInputFile.h"

#include "dcast.h"
#include "indent.h"

TypeHandle LwoVertexMap::_type_handle;


/**
 * Returns true if the map has a value associated with the given index, false
 * otherwise.
 */
bool LwoVertexMap::
has_value(int index) const {
  return (_vmap.count(index) != 0);
}

/**
 * Returns the mapping value associated with the given index, or an empty
 * PTA_stdfloat if there is no mapping value associated.
 */
PTA_stdfloat LwoVertexMap::
get_value(int index) const {
  VMap::const_iterator vi;
  vi = _vmap.find(index);
  if (vi != _vmap.end()) {
    return (*vi).second;
  }

  return PTA_stdfloat();
}

/**
 * Reads the data of the chunk in from the given input file, if possible.  The
 * ID and length of the chunk have already been read.  stop_at is the byte
 * position of the file to stop at (based on the current position at
 * in->get_bytes_read()).  Returns true on success, false otherwise.
 */
bool LwoVertexMap::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  _map_type = lin->get_id();
  _dimension = lin->get_be_uint16();
  _name = lin->get_string();

  while (lin->get_bytes_read() < stop_at && !lin->is_eof()) {
    int index = lin->get_vx();

    PTA_stdfloat value;
    for (int i = 0; i < _dimension; i++) {
      value.push_back(lin->get_be_float32());
    }

    bool inserted = _vmap.insert(VMap::value_type(index, value)).second;
    if (!inserted) {
      nout << "Duplicate index " << index << " in map.\n";
    }
  }

  return (lin->get_bytes_read() == stop_at);
}

/**
 *
 */
void LwoVertexMap::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " { map_type = " << _map_type
    << ", dimension = " << _dimension
    << ", name = \"" << _name << "\", "
    << _vmap.size() << " values }\n";
}
