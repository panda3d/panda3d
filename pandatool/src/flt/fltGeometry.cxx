// Filename: fltGeometry.cxx
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

#include "fltGeometry.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"
#include "fltHeader.h"
#include "fltMaterial.h"

TypeHandle FltGeometry::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltGeometry::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltGeometry::
FltGeometry(FltHeader *header) : FltBeadID(header) {
  _ir_color = 0;
  _relative_priority = 0;
  _draw_type = DT_solid_cull_backface;
  _texwhite = false;
  _color_name_index = 0;
  _alt_color_name_index = 0;
  _billboard_type = BT_none;
  _detail_texture_index = -1;
  _texture_index = -1;
  _material_index = -1;
  _dfad_material_code = 0;
  _dfad_feature_id = 0;
  _ir_material_code = 0;
  _transparency = 0;
  _lod_generation_control = 0;
  _line_style_index = 0;
  _flags = F_no_color;
  _light_mode = LM_face_no_normal;
  _texture_mapping_index = 0;
  _color_index = 0;
  _alt_color_index = 0;
}


////////////////////////////////////////////////////////////////////
//     Function: FltGeometry::get_color
//       Access: Public
//  Description: Returns the primary color of the face, as a
//               four-component value (including alpha as the
//               transparency channel).
//
//               If has_color() is false, the result is white, but
//               still reflects the transparency correctly.
////////////////////////////////////////////////////////////////////
LColor FltGeometry::
get_color() const {
  LColor color;

  if (!has_color() || (_texwhite && has_texture())) {
    // Force this one white.
    color.set(1.0, 1.0, 1.0, 1.0);

  } else if (has_material()) {
    // If we have a material, that replaces the color.
    FltMaterial *material = get_material();
    color.set(material->_diffuse[0],
              material->_diffuse[1],
              material->_diffuse[2],
              material->_alpha);
  } else {
    LRGBColor rgb =
      _header->get_rgb(_color_index, (_flags & F_packed_color) != 0,
                       _packed_color);
    color.set(rgb[0], rgb[1], rgb[2], 1.0);
  }

  // Modify the whole thing by our transparency.
  PN_stdfloat alpha = 1.0 - (_transparency / 65535.0);
  color[3] *= alpha;

  return color;
}

////////////////////////////////////////////////////////////////////
//     Function: FltGeometry::set_color
//       Access: Public
//  Description: Sets the primary color of the face, using the packed
//               color convention.
////////////////////////////////////////////////////////////////////
void FltGeometry::
set_color(const LColor &color) {
  set_rgb(LRGBColor(color[0], color[1], color[2]));
  _transparency = (int)floor((1.0 - color[3]) * 65535.0);
}

////////////////////////////////////////////////////////////////////
//     Function: FltGeometry::get_rgb
//       Access: Public
//  Description: Returns the primary color of the face, as a
//               three-component value ignoring transparency.
////////////////////////////////////////////////////////////////////
LRGBColor FltGeometry::
get_rgb() const {
  if (!has_color() || (_texwhite && has_texture())) {
    // Force this one white.
    return LRGBColor(1.0, 1.0, 1.0);
  }

  if (has_material()) {
    // If we have a material, that replaces the color.
    FltMaterial *material = get_material();
    return material->_diffuse;
  }

  return _header->get_rgb(_color_index, (_flags & F_packed_color) != 0,
                          _packed_color);
}

////////////////////////////////////////////////////////////////////
//     Function: FltGeometry::set_rgb
//       Access: Public
//  Description: Sets the primary color of the face, using the packed
//               color convention; does not affect transparency.
////////////////////////////////////////////////////////////////////
void FltGeometry::
set_rgb(const LRGBColor &rgb) {
  _packed_color.set_rgb(rgb);
  _flags = ((_flags & ~F_no_color) | F_packed_color);

  // If we have a color, we can't have a material.
  _material_index = -1;
  _texwhite = false;
}

////////////////////////////////////////////////////////////////////
//     Function: FltGeometry::has_alt_color
//       Access: Public
//  Description: Returns true if the face has an alternate color
//               indicated, false otherwise.
////////////////////////////////////////////////////////////////////
bool FltGeometry::
has_alt_color() const {
  return (_flags & F_no_alt_color) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FltGeometry::get_alt_color
//       Access: Public
//  Description: If has_alt_color() indicates true, returns the alternate
//               color of the face, as a four-component value
//               (including alpha as the transparency channel).
////////////////////////////////////////////////////////////////////
LColor FltGeometry::
get_alt_color() const {
  nassertr(has_alt_color(), LColor(0.0, 0.0, 0.0, 0.0));

  return _header->get_color(_alt_color_index, (_flags & F_packed_color) != 0,
                            _alt_packed_color, _transparency);
}

////////////////////////////////////////////////////////////////////
//     Function: FltGeometry::get_alt_rgb
//       Access: Public
//  Description: If has_alt_color() indicates true, returns the alternate
//               color of the face, as a three-component value
//               ignoring transparency.
////////////////////////////////////////////////////////////////////
LRGBColor FltGeometry::
get_alt_rgb() const {
  nassertr(has_alt_color(), LRGBColor(0.0, 0.0, 0.0));

  return _header->get_rgb(_alt_color_index, (_flags & F_packed_color) != 0,
                          _alt_packed_color);
}

////////////////////////////////////////////////////////////////////
//     Function: FltGeometry::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this bead based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltGeometry::
extract_record(FltRecordReader &reader) {
  DatagramIterator &iterator = reader.get_iterator();

  _ir_color = iterator.get_be_int32();
  _relative_priority = iterator.get_be_int16();
  _draw_type = (DrawType)iterator.get_int8();
  _texwhite = (iterator.get_int8() != 0);
  _color_name_index = iterator.get_be_int16();
  _alt_color_name_index = iterator.get_be_int16();
  iterator.skip_bytes(1);
  _billboard_type = (BillboardType)iterator.get_int8();
  _detail_texture_index = iterator.get_be_int16();
  _texture_index = iterator.get_be_int16();
  _material_index = iterator.get_be_int16();
  _dfad_material_code = iterator.get_be_int16();
  _dfad_feature_id = iterator.get_be_int16();
  _ir_material_code = iterator.get_be_int32();
  _transparency = iterator.get_be_uint16();
  _lod_generation_control = iterator.get_uint8();
  _line_style_index = iterator.get_uint8();
  if (_header->get_flt_version() >= 1420) {
    _flags = iterator.get_be_uint32();
    _light_mode = (LightMode)iterator.get_uint8();
    iterator.skip_bytes(1 + 4);
    iterator.skip_bytes(2); // Undocumented padding.

    if (!_packed_color.extract_record(reader)) {
      return false;
    }
    if (!_alt_packed_color.extract_record(reader)) {
      return false;
    }
    
    if (_header->get_flt_version() >= 1520) {
      _texture_mapping_index = iterator.get_be_int16();
      iterator.skip_bytes(2);
      _color_index = iterator.get_be_int32();
      _alt_color_index = iterator.get_be_int32();
      iterator.skip_bytes(2 + 2);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltGeometry::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltGeometry::
build_record(FltRecordWriter &writer) const {
  Datagram &datagram = writer.update_datagram();

  datagram.add_be_int32(_ir_color);
  datagram.add_be_int16(_relative_priority);
  datagram.add_int8(_draw_type);
  datagram.add_int8(_texwhite);
  datagram.add_be_uint16(_color_name_index);
  datagram.add_be_uint16(_alt_color_name_index);
  datagram.pad_bytes(1);
  datagram.add_int8(_billboard_type);
  datagram.add_be_int16(_detail_texture_index);
  datagram.add_be_int16(_texture_index);
  datagram.add_be_int16(_material_index);
  datagram.add_be_int16(_dfad_material_code);
  datagram.add_be_int16(_dfad_feature_id);
  datagram.add_be_int32(_ir_material_code);
  datagram.add_be_uint16(_transparency);
  datagram.add_uint8(_lod_generation_control);
  datagram.add_uint8(_line_style_index);
  datagram.add_be_uint32(_flags);
  datagram.add_uint8(_light_mode);
  datagram.pad_bytes(1 + 4);
  datagram.pad_bytes(2); // Undocumented padding.

  if (!_packed_color.build_record(writer)) {
    return false;
  }
  if (!_alt_packed_color.build_record(writer)) {
    return false;
  }

  if (_header->get_flt_version() >= 1520) {
    // New with 15.2
    datagram.add_be_int16(_texture_mapping_index);
    datagram.pad_bytes(2);
    datagram.add_be_int32(_color_index);
    datagram.add_be_int32(_alt_color_index);
    datagram.pad_bytes(2 + 2);
  }

  return true;
}
