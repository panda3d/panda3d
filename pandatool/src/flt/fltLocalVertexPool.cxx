// Filename: fltLocalVertexPool.cxx
// Created by:  drose (28Feb01)
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

#include "fltLocalVertexPool.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"
#include "fltHeader.h"
#include "fltMaterial.h"

TypeHandle FltLocalVertexPool::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltLocalVertexPool::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltLocalVertexPool::
FltLocalVertexPool(FltHeader *header) : FltRecord(header) {
}

////////////////////////////////////////////////////////////////////
//     Function: FltLocalVertexPool::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this bead based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltLocalVertexPool::
extract_record(FltRecordReader &reader) {
  if (!FltRecord::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_local_vertex_pool, false);
  DatagramIterator &iterator = reader.get_iterator();

  int num_vertices = iterator.get_be_int32();
  int attributes = iterator.get_be_int32();

  for (int i = 0; i < num_vertices; i++) {
    FltVertex *vertex = new FltVertex(_header);
    _vertices.push_back(vertex);

    if ((attributes & AM_has_position) != 0) {
      vertex->_pos[0] = iterator.get_be_float64();
      vertex->_pos[1] = iterator.get_be_float64();
      vertex->_pos[2] = iterator.get_be_float64();
    }

    if ((attributes & AM_has_color_index) != 0) {
      vertex->_color_index = iterator.get_be_int32();

    } else if ((attributes & AM_has_packed_color) != 0) {
      if (!vertex->_packed_color.extract_record(reader)) {
        return false;
      }
      vertex->_flags |= FltVertex::F_packed_color;

    } else {
      vertex->_flags |= FltVertex::F_no_color;
    }

    if ((attributes & AM_has_normal) != 0) {
      vertex->_normal[0] = iterator.get_be_float32();
      vertex->_normal[1] = iterator.get_be_float32();
      vertex->_normal[2] = iterator.get_be_float32();
      vertex->_has_normal = true;
    }

    if ((attributes & AM_has_base_uv) != 0) {
      vertex->_uv[0] = iterator.get_be_float32();
      vertex->_uv[1] = iterator.get_be_float32();
      vertex->_has_uv = true;
    }

    if ((attributes & AM_has_uv_1) != 0) {
      iterator.get_be_float32();
      iterator.get_be_float32();
    }

    if ((attributes & AM_has_uv_2) != 0) {
      iterator.get_be_float32();
      iterator.get_be_float32();
    }

    if ((attributes & AM_has_uv_3) != 0) {
      iterator.get_be_float32();
      iterator.get_be_float32();
    }

    if ((attributes & AM_has_uv_4) != 0) {
      iterator.get_be_float32();
      iterator.get_be_float32();
    }

    if ((attributes & AM_has_uv_5) != 0) {
      iterator.get_be_float32();
      iterator.get_be_float32();
    }

    if ((attributes & AM_has_uv_6) != 0) {
      iterator.get_be_float32();
      iterator.get_be_float32();
    }

    if ((attributes & AM_has_uv_7) != 0) {
      iterator.get_be_float32();
      iterator.get_be_float32();
    }
  }

  check_remaining_size(iterator);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltLocalVertexPool::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltLocalVertexPool::
build_record(FltRecordWriter &writer) const {
  if (!FltRecord::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_local_vertex_pool);
  Datagram &datagram = writer.update_datagram();

  // Determine what kind of vertices we have.
  int attributes = AM_has_position;

  Vertices::const_iterator vi;
  for (vi = _vertices.begin(); vi != _vertices.end(); ++vi) {
    FltVertex *vertex = (*vi);
    if ((vertex->_flags & FltVertex::F_no_color) != 0) {
      // No color.

    } else if ((vertex->_flags & FltVertex::F_packed_color) != 0) {
      // Packed color.
      attributes |= AM_has_packed_color;

    } else {
      // Indexed color.
      attributes |= AM_has_color_index;
    }

    if (vertex->_has_normal) {
      attributes |= AM_has_normal;
    }

    if (vertex->_has_uv) {
      attributes |= AM_has_base_uv;
    }
  }

  if ((attributes & AM_has_packed_color) != 0 &&
      (attributes & AM_has_color_index) != 0) {
    // We cannot have both a packed color and a color index.  If we
    // want both, used packed color.
    attributes &= ~AM_has_color_index;
  }

  datagram.add_be_int32(_vertices.size());
  datagram.add_be_int32(attributes);

  // Now write out each vertex.
  for (vi = _vertices.begin(); vi != _vertices.end(); ++vi) {
    FltVertex *vertex = (*vi);

    if ((attributes & AM_has_position) != 0) {
      datagram.add_be_float64(vertex->_pos[0]);
      datagram.add_be_float64(vertex->_pos[1]);
      datagram.add_be_float64(vertex->_pos[2]);
    }

    if ((attributes & AM_has_color_index) != 0) {
      if ((vertex->_flags & (FltVertex::F_no_color | FltVertex::F_packed_color)) != 0) {
        // This particular vertex does not have a color index.
        // Make it white.
        datagram.add_be_int32(_header->get_closest_rgb(LRGBColor(1.0, 1.0, 1.0)));
      } else {
        datagram.add_be_int32(vertex->_color_index);
      }

    } else if ((attributes & AM_has_packed_color) != 0) {
      // We extract our own FltPackedColor instead of writing out the
      // vertex's _packed_color directly, just in case the vertex is
      // actually index colored.  This bit of code will work
      // regardless of the kind of color the vertex has.

      FltPackedColor color;
      if (vertex->has_color()) {
        color.set_color(vertex->get_color());
      } else {
        // An uncolored vertex.  Make it white.
        color.set_color(LColor(1.0, 1.0, 1.0, 1.0));
      }

      if (!color.build_record(writer)) {
        return false;
      }
    }

    if ((attributes & AM_has_normal) != 0) {
      if (!vertex->_has_normal) {
        datagram.add_be_float32(0.0);
        datagram.add_be_float32(0.0);
        datagram.add_be_float32(0.0);
      } else {
        datagram.add_be_float32(vertex->_normal[0]);
        datagram.add_be_float32(vertex->_normal[1]);
        datagram.add_be_float32(vertex->_normal[2]);
      }
    }

    if ((attributes & AM_has_base_uv) != 0) {
      if (!vertex->_has_uv) {
        datagram.add_be_float32(0.0);
        datagram.add_be_float32(0.0);
      } else {
        datagram.add_be_float32(vertex->_uv[0]);
        datagram.add_be_float32(vertex->_uv[1]);
      }
    }
  }

  return true;
}
