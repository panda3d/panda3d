// Filename: fltMaterial.cxx
// Created by:  drose (25Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "fltMaterial.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

TypeHandle FltMaterial::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltMaterial::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FltMaterial::
FltMaterial(FltHeader *header) : FltRecord(header) {
  _material_index = 0;
  _flags = 0;
  _ambient.set(0.0, 0.0, 0.0);
  _diffuse.set(0.0, 0.0, 0.0);
  _specular.set(0.0, 0.0, 0.0);
  _emissive.set(0.0, 0.0, 0.0);
  _shininess = 0.0;
  _alpha = 1.0;
}

////////////////////////////////////////////////////////////////////
//     Function: FltMaterial::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this record based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltMaterial::
extract_record(FltRecordReader &reader) {
  if (!FltRecord::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_15_material, false);
  DatagramIterator &iterator = reader.get_iterator();

  _material_index = iterator.get_be_int32();
  _material_name = iterator.get_fixed_string(12);
  _flags = iterator.get_be_uint32();
  _ambient[0] = iterator.get_be_float32();
  _ambient[1] = iterator.get_be_float32();
  _ambient[2] = iterator.get_be_float32();
  _diffuse[0] = iterator.get_be_float32();
  _diffuse[1] = iterator.get_be_float32();
  _diffuse[2] = iterator.get_be_float32();
  _specular[0] = iterator.get_be_float32();
  _specular[1] = iterator.get_be_float32();
  _specular[2] = iterator.get_be_float32();
  _emissive[0] = iterator.get_be_float32();
  _emissive[1] = iterator.get_be_float32();
  _emissive[2] = iterator.get_be_float32();
  _shininess = iterator.get_be_float32();
  _alpha = iterator.get_be_float32();
  iterator.skip_bytes(4);

  check_remaining_size(iterator);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltMaterial::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltMaterial::
build_record(FltRecordWriter &writer) const {
  if (!FltRecord::build_record(writer)) {
    return false;
  }
  
  writer.set_opcode(FO_15_material);
  Datagram &datagram = writer.update_datagram();

  datagram.add_be_int32(_material_index);
  datagram.add_fixed_string(_material_name, 12);
  datagram.add_be_uint32(_flags);
  datagram.add_be_float32(_ambient[0]);
  datagram.add_be_float32(_ambient[1]);
  datagram.add_be_float32(_ambient[2]);
  datagram.add_be_float32(_diffuse[0]);
  datagram.add_be_float32(_diffuse[1]);
  datagram.add_be_float32(_diffuse[2]);
  datagram.add_be_float32(_specular[0]);
  datagram.add_be_float32(_specular[1]);
  datagram.add_be_float32(_specular[2]);
  datagram.add_be_float32(_emissive[0]);
  datagram.add_be_float32(_emissive[1]);
  datagram.add_be_float32(_emissive[2]);
  datagram.add_be_float32(_shininess);
  datagram.add_be_float32(_alpha);
  datagram.pad_bytes(4);

  return true;
}
