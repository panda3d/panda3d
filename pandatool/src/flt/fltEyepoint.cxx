// Filename: fltEyepoint.cxx
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

#include "fltEyepoint.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

////////////////////////////////////////////////////////////////////
//     Function: FltEyepoint::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltEyepoint::
FltEyepoint() {
  _rotation_center.set(0.0, 0.0, 0.0);
  _hpr.set(0.0, 0.0, 0.0);
  _rotation = LMatrix4::ident_mat();
  _fov = 60.0;
  _scale = 1.0;
  _near_clip = 0.1;
  _far_clip = 10000.0;
  _fly_through = LMatrix4::ident_mat();
  _eyepoint.set(0.0, 0.0, 0.0);
  _fly_through_yaw = 0.0;
  _fly_through_pitch = 0.0;
  _eyepoint_direction.set(0.0, 1.0, 0.0);
  _no_fly_through = true;
  _ortho_mode = false;
  _is_valid = true;
  _image_offset_x = 0;
  _image_offset_y = 0;
  _image_zoom = 1;
}

////////////////////////////////////////////////////////////////////
//     Function: FltEyepoint::extract_record
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool FltEyepoint::
extract_record(FltRecordReader &reader) {
  DatagramIterator &iterator = reader.get_iterator();

  _rotation_center[0] = iterator.get_be_float64();
  _rotation_center[1] = iterator.get_be_float64();
  _rotation_center[2] = iterator.get_be_float64();
  _hpr[0] = iterator.get_be_float32();
  _hpr[1] = iterator.get_be_float32();
  _hpr[2] = iterator.get_be_float32();
  int r;
  for (r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      _rotation(r, c) = iterator.get_be_float32();
    }
  }
  _fov = iterator.get_be_float32();
  _scale = iterator.get_be_float32();
  _near_clip = iterator.get_be_float32();
  _far_clip = iterator.get_be_float32();
  for (r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      _fly_through(r, c) = iterator.get_be_float32();
    }
  }
  _eyepoint[0] = iterator.get_be_float32();
  _eyepoint[1] = iterator.get_be_float32();
  _eyepoint[2] = iterator.get_be_float32();
  _fly_through_yaw = iterator.get_be_float32();
  _fly_through_pitch = iterator.get_be_float32();
  _eyepoint_direction[0] = iterator.get_be_float32();
  _eyepoint_direction[1] = iterator.get_be_float32();
  _eyepoint_direction[2] = iterator.get_be_float32();
  _no_fly_through = (iterator.get_be_int32() != 0);
  _ortho_mode = (iterator.get_be_int32() != 0);
  _is_valid = (iterator.get_be_int32() != 0);
  _image_offset_x = iterator.get_be_int32();
  _image_offset_y = iterator.get_be_int32();
  _image_zoom = iterator.get_be_int32();
  iterator.skip_bytes(4*9);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltEyepoint::build_record
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool FltEyepoint::
build_record(FltRecordWriter &writer) const {
  Datagram &datagram = writer.update_datagram();

  datagram.add_be_float64(_rotation_center[0]);
  datagram.add_be_float64(_rotation_center[1]);
  datagram.add_be_float64(_rotation_center[2]);
  datagram.add_be_float32(_hpr[0]);
  datagram.add_be_float32(_hpr[1]);
  datagram.add_be_float32(_hpr[2]);
  int r;
  for (r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      datagram.add_be_float32(_rotation(r, c));
    }
  }
  datagram.add_be_float32(_fov);
  datagram.add_be_float32(_scale);
  datagram.add_be_float32(_near_clip);
  datagram.add_be_float32(_far_clip);
  for (r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      datagram.add_be_float32(_fly_through(r, c));
    }
  }
  datagram.add_be_float32(_eyepoint[0]);
  datagram.add_be_float32(_eyepoint[1]);
  datagram.add_be_float32(_eyepoint[2]);
  datagram.add_be_float32(_fly_through_yaw);
  datagram.add_be_float32(_fly_through_pitch);
  datagram.add_be_float32(_eyepoint_direction[0]);
  datagram.add_be_float32(_eyepoint_direction[1]);
  datagram.add_be_float32(_eyepoint_direction[2]);
  datagram.add_be_int32(_no_fly_through);
  datagram.add_be_int32(_ortho_mode);
  datagram.add_be_int32(_is_valid);
  datagram.add_be_int32(_image_offset_x);
  datagram.add_be_int32(_image_offset_y);
  datagram.add_be_int32(_image_zoom);
  datagram.pad_bytes(4*9);

  return true;
}
