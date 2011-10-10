// Filename: fltVertex.cxx
// Created by:  drose (25Aug00)
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

#include "fltVertex.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"
#include "fltHeader.h"

TypeHandle FltVertex::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltVertex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltVertex::
FltVertex(FltHeader *header) : FltRecord(header) {
  _color_name_index = 0;
  _flags = F_no_color;
  _pos.set(0.0, 0.0, 0.0);
  _normal.set(0.0, 0.0, 0.0);
  _uv.set(0.0, 0.0);
  _color_index = 0;

  _has_normal = false;
  _has_uv = false;
}

////////////////////////////////////////////////////////////////////
//     Function: FltVertex::get_opcode
//       Access: Public
//  Description: Returns the opcode that this record will be written
//               as.
////////////////////////////////////////////////////////////////////
FltOpcode FltVertex::
get_opcode() const {
  if (_has_normal) {
    if (_has_uv) {
      return FO_vertex_cnu;
    } else {
      return FO_vertex_cn;
    }
  } else {
    if (_has_uv) {
      return FO_vertex_cu;
    } else {
      return FO_vertex_c;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FltVertex::get_record_length
//       Access: Public
//  Description: Returns the length of this record in bytes as it will
//               be written to the flt file.
////////////////////////////////////////////////////////////////////
int FltVertex::
get_record_length() const {
  if (_header->get_flt_version() < 1520) {
    // Version 14.2
    switch (get_opcode()) {
    case FO_vertex_c:
      return 36;

    case FO_vertex_cn:
      return 48;

    case FO_vertex_cnu:
      return 56;

    case FO_vertex_cu:
      return 44;

    default:
      nassertr(false, 0);
    }

  } else {
    // Version 15.2 and higher
    switch (get_opcode()) {
    case FO_vertex_c:
      return 40;

    case FO_vertex_cn:
      return 56;

    case FO_vertex_cnu:
      return 64;

    case FO_vertex_cu:
      return 48;

    default:
      nassertr(false, 0);
    }
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FltVertex::get_color
//       Access: Public
//  Description: If has_color() indicates true, returns the 
//               color of the vertex, as a four-component value.  In
//               the case of a vertex, the alpha channel will always
//               be 1.0, as MultiGen does not store transparency
//               per-vertex.
////////////////////////////////////////////////////////////////////
LColor FltVertex::
get_color() const {
  nassertr(has_color(), LColor(0.0, 0.0, 0.0, 0.0));

  return _header->get_color(_color_index, (_flags & F_packed_color) != 0,
                            _packed_color, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: FltVertex::get_rgb
//       Access: Public
//  Description: If has_color() indicates true, returns the 
//               color of the vertex, as a three-component value.
////////////////////////////////////////////////////////////////////
LRGBColor FltVertex::
get_rgb() const {
  nassertr(has_color(), LRGBColor(0.0, 0.0, 0.0));

  return _header->get_rgb(_color_index, (_flags & F_packed_color) != 0,
                          _packed_color);
}

////////////////////////////////////////////////////////////////////
//     Function: FltVertex::set_rgb
//       Access: Public
//  Description: Sets the color of the vertex, using the packed
//               color convention.
////////////////////////////////////////////////////////////////////
void FltVertex::
set_rgb(const LRGBColor &rgb) {
  _packed_color.set_rgb(rgb);
  _flags = ((_flags & ~F_no_color) | F_packed_color);
}

////////////////////////////////////////////////////////////////////
//     Function: FltVertex::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this record based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltVertex::
extract_record(FltRecordReader &reader) {
  if (!FltRecord::extract_record(reader)) {
    return false;
  }

  switch (reader.get_opcode()) {
  case FO_vertex_c:
    _has_normal = false;
    _has_uv = false;
    break;

  case FO_vertex_cn:
    _has_normal = true;
    _has_uv = false;
    break;

  case FO_vertex_cnu:
    _has_normal = true;
    _has_uv = true;
    break;

  case FO_vertex_cu:
    _has_normal = false;
    _has_uv = true;
    break;

  default:
    nassertr(false, false);
  }

  DatagramIterator &iterator = reader.get_iterator();

  _color_name_index = iterator.get_be_int16();
  _flags = iterator.get_be_uint16();
  _pos[0] = iterator.get_be_float64();
  _pos[1] = iterator.get_be_float64();
  _pos[2] = iterator.get_be_float64();

  if (_has_normal) {
    _normal[0] = iterator.get_be_float32();
    _normal[1] = iterator.get_be_float32();
    _normal[2] = iterator.get_be_float32();
  }
  if (_has_uv) {
    _uv[0] = iterator.get_be_float32();
    _uv[1] = iterator.get_be_float32();
  }

  if (iterator.get_remaining_size() > 0) {
    if (!_packed_color.extract_record(reader)) {
      return false;
    }
    if (_header->get_flt_version() >= 1520) {
      _color_index = iterator.get_be_int32();
      
      if (_has_normal && iterator.get_remaining_size() > 0) {
        // If we extracted a normal, our double-word alignment is off; now
        // we have a few extra bytes to ignore.
        iterator.skip_bytes(4);
      }
    }
  }

  check_remaining_size(iterator);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltVertex::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltVertex::
build_record(FltRecordWriter &writer) const {
  if (!FltRecord::build_record(writer)) {
    return false;
  }

  writer.set_opcode(get_opcode());
  Datagram &datagram = writer.update_datagram();

  datagram.add_be_int16(_color_name_index);
  datagram.add_be_uint16(_flags);
  datagram.add_be_float64(_pos[0]);
  datagram.add_be_float64(_pos[1]);
  datagram.add_be_float64(_pos[2]);

  if (_has_normal) {
    datagram.add_be_float32(_normal[0]);
    datagram.add_be_float32(_normal[1]);
    datagram.add_be_float32(_normal[2]);
  }
  if (_has_uv) {
    datagram.add_be_float32(_uv[0]);
    datagram.add_be_float32(_uv[1]);
  }

  if (!_packed_color.build_record(writer)) {
    return false;
  }

  if (_header->get_flt_version() >= 1520) {
    // New with 15.2
    datagram.add_be_uint32(_color_index);

    if (_has_normal) {
      // If we added a normal, our double-word alignment is off; now we
      // have a few extra bytes to add.
      datagram.pad_bytes(4);
    }
  }

  nassertr((int)datagram.get_length() == get_record_length() - 4, true);
  return true;
}
