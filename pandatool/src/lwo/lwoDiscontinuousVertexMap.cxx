/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoDiscontinuousVertexMap.cxx
 * @author drose
 * @date 2001-04-24
 */

#include "lwoDiscontinuousVertexMap.h"
#include "lwoInputFile.h"

#include "dcast.h"
#include "indent.h"

#include <algorithm>

TypeHandle LwoDiscontinuousVertexMap::_type_handle;


/**
 * Returns true if the map has a value associated with the given index, false
 * otherwise.
 */
bool LwoDiscontinuousVertexMap::
has_value(int polygon_index, int vertex_index) const {
  VMad::const_iterator di;
  di = _vmad.find(polygon_index);
  if (di != _vmad.end()) {
    const VMap &vmap = (*di).second;
    return (vmap.count(vertex_index) != 0);
  }

  return false;
}

/**
 * Returns the mapping value associated with the given index, or an empty
 * PTA_stdfloat if there is no mapping value associated.
 */
PTA_stdfloat LwoDiscontinuousVertexMap::
get_value(int polygon_index, int vertex_index) const {
  VMad::const_iterator di;
  di = _vmad.find(polygon_index);
  if (di != _vmad.end()) {
    const VMap &vmap = (*di).second;
    VMap::const_iterator vi;
    vi = vmap.find(vertex_index);
    if (vi != vmap.end()) {
      return (*vi).second;
    }
  }

  return PTA_stdfloat();
}

/**
 * Reads the data of the chunk in from the given input file, if possible.  The
 * ID and length of the chunk have already been read.  stop_at is the byte
 * position of the file to stop at (based on the current position at
 * in->get_bytes_read()).  Returns true on success, false otherwise.
 */
bool LwoDiscontinuousVertexMap::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  _map_type = lin->get_id();
  _dimension = lin->get_be_uint16();
  _name = lin->get_string();

  while (lin->get_bytes_read() < stop_at && !lin->is_eof()) {
    int vertex_index = lin->get_vx();
    int polygon_index = lin->get_vx();

    PTA_stdfloat value;
    for (int i = 0; i < _dimension; i++) {
      value.push_back(lin->get_be_float32());
    }

    VMap &vmap = _vmad[polygon_index];
    std::pair<VMap::iterator, bool> ir =
      vmap.insert(VMap::value_type(vertex_index, value));
    if (!ir.second) {
      // This polygonvertex pair was repeated in the vmad.  Is it simply
      // redundant, or is it contradictory?
      PTA_stdfloat orig_value = (*ir.first).second;

      if (value.v() != orig_value.v()) {
        nout << "Multiple UV values for vertex " << vertex_index
             << " of polygon " << polygon_index
             << " specified by discontinuous vertex map.\n"
             << "Original value = ";

        PTA_stdfloat::const_iterator vi;
        for (vi = orig_value.begin(); vi != orig_value.end(); ++vi) {
          nout << (*vi) << " ";
        }
        nout << " new value = ";
        for (vi = value.begin(); vi != value.end(); ++vi) {
          nout << (*vi) << " ";
        }
        nout << "\n";
      }
    }
  }

  return (lin->get_bytes_read() == stop_at);
}

/**
 *
 */
void LwoDiscontinuousVertexMap::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " { map_type = " << _map_type
    << ", dimension = " << _dimension
    << ", name = \"" << _name << "\", "
    << _vmad.size() << " polygons }\n";
}
