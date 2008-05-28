// Filename: fltLightSourceDefinition.cxx
// Created by:  drose (26Aug00)
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

#include "fltLightSourceDefinition.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

TypeHandle FltLightSourceDefinition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltLightSourceDefinition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltLightSourceDefinition::
FltLightSourceDefinition(FltHeader *header) : FltRecord(header) {
  _light_index = 0;
  _ambient.set(0.0, 0.0, 0.0, 1.0);
  _diffuse.set(1.0, 1.0, 1.0, 1.0);
  _specular.set(0.0, 0.0, 0.0, 1.0);
  _light_type = LT_infinite;
  _exponential_dropoff = 1.0;
  _cutoff_angle = 180.0;
  _yaw = 0.0;
  _pitch = 0.0;
  _constant_coefficient = 0.0;
  _linear_coefficient = 0.0;
  _quadratic_coefficient = 1.0;
  _modeling_light = false;
}

////////////////////////////////////////////////////////////////////
//     Function: FltLightSourceDefinition::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this record based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltLightSourceDefinition::
extract_record(FltRecordReader &reader) {
  if (!FltRecord::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_light_definition, false);
  DatagramIterator &iterator = reader.get_iterator();

  _light_index = iterator.get_be_int32();
  iterator.skip_bytes(2*4);
  _light_name = iterator.get_fixed_string(20);
  iterator.skip_bytes(4);
  _ambient[0] = iterator.get_be_float32();
  _ambient[1] = iterator.get_be_float32();
  _ambient[2] = iterator.get_be_float32();
  _ambient[3] = iterator.get_be_float32();
  _diffuse[0] = iterator.get_be_float32();
  _diffuse[1] = iterator.get_be_float32();
  _diffuse[2] = iterator.get_be_float32();
  _diffuse[3] = iterator.get_be_float32();
  _specular[0] = iterator.get_be_float32();
  _specular[1] = iterator.get_be_float32();
  _specular[2] = iterator.get_be_float32();
  _specular[3] = iterator.get_be_float32();
  _light_type = (LightType)iterator.get_be_int32();
  iterator.skip_bytes(4*10);
  _exponential_dropoff = iterator.get_be_float32();
  _cutoff_angle = iterator.get_be_float32();
  _yaw = iterator.get_be_float32();
  _pitch = iterator.get_be_float32();
  _constant_coefficient = iterator.get_be_float32();
  _linear_coefficient = iterator.get_be_float32();
  _quadratic_coefficient = iterator.get_be_float32();
  _modeling_light = (iterator.get_be_int32() != 0);
  iterator.skip_bytes(4*19);

  check_remaining_size(iterator);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltLightSourceDefinition::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltLightSourceDefinition::
build_record(FltRecordWriter &writer) const {
  if (!FltRecord::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_light_definition);
  Datagram &datagram = writer.update_datagram();

  datagram.add_be_int32(_light_index);
  datagram.pad_bytes(2*4);
  datagram.add_fixed_string(_light_name, 20);
  datagram.pad_bytes(4);
  datagram.add_be_float32(_ambient[0]);
  datagram.add_be_float32(_ambient[1]);
  datagram.add_be_float32(_ambient[2]);
  datagram.add_be_float32(_ambient[3]);
  datagram.add_be_float32(_diffuse[0]);
  datagram.add_be_float32(_diffuse[1]);
  datagram.add_be_float32(_diffuse[2]);
  datagram.add_be_float32(_diffuse[3]);
  datagram.add_be_float32(_specular[0]);
  datagram.add_be_float32(_specular[1]);
  datagram.add_be_float32(_specular[2]);
  datagram.add_be_float32(_specular[3]);
  datagram.add_be_int32(_light_type);
  datagram.pad_bytes(4*10);
  datagram.add_be_float32(_exponential_dropoff);
  datagram.add_be_float32(_cutoff_angle);
  datagram.add_be_float32(_yaw);
  datagram.add_be_float32(_pitch);
  datagram.add_be_float32(_constant_coefficient);
  datagram.add_be_float32(_linear_coefficient);
  datagram.add_be_float32(_quadratic_coefficient);
  datagram.add_be_int32(_modeling_light);
  datagram.pad_bytes(4*19);

  return true;
}
