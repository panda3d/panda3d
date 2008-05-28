// Filename: fltTrackplane.cxx
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

#include "fltTrackplane.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

////////////////////////////////////////////////////////////////////
//     Function: FltTrackplane::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltTrackplane::
FltTrackplane() {
  _origin.set(0.0, 0.0, 0.0);
  _alignment.set(0.0, 0.0, 0.0);
  _plane.set(0.0, 0.0, 1.0);
  _grid_state = false;
  _grid_under = false;
  _grid_angle = 0.0;
  _grid_spacing_x = 1;
  _grid_spacing_y = 1;
  _snap_to_grid = false;
  _grid_size = 10.0;
  _grid_spacing_direction = 0;
  _grid_mask = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTrackplane::extract_record
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool FltTrackplane::
extract_record(FltRecordReader &reader) {
  DatagramIterator &iterator = reader.get_iterator();

  _origin[0] = iterator.get_be_float64();
  _origin[1] = iterator.get_be_float64();
  _origin[2] = iterator.get_be_float64();
  _alignment[0] = iterator.get_be_float64();
  _alignment[1] = iterator.get_be_float64();
  _alignment[0] = iterator.get_be_float64();
  _plane[0] = iterator.get_be_float64();
  _plane[1] = iterator.get_be_float64();
  _plane[2] = iterator.get_be_float64();
  _grid_state = (iterator.get_be_int32() != 0);
  _grid_under = (iterator.get_be_int32() != 0);
  _grid_angle = iterator.get_be_float32();
  iterator.skip_bytes(4);
  _grid_spacing_x = iterator.get_be_float64();
  _grid_spacing_y = iterator.get_be_float64();
  _snap_to_grid = (iterator.get_be_int32() != 0);
  _grid_size = iterator.get_be_float64();
  _grid_spacing_direction = iterator.get_be_int32();
  _grid_mask = iterator.get_be_int32();
  iterator.skip_bytes(4);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTrackplane::build_record
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool FltTrackplane::
build_record(FltRecordWriter &writer) const {
  Datagram &datagram = writer.update_datagram();

  datagram.add_be_float64(_origin[0]);
  datagram.add_be_float64(_origin[1]);
  datagram.add_be_float64(_origin[2]);
  datagram.add_be_float64(_alignment[0]);
  datagram.add_be_float64(_alignment[1]);
  datagram.add_be_float64(_alignment[2]);
  datagram.add_be_float64(_plane[0]);
  datagram.add_be_float64(_plane[1]);
  datagram.add_be_float64(_plane[2]);
  datagram.add_be_int32(_grid_state);
  datagram.add_be_int32(_grid_under);
  datagram.add_be_float32(_grid_angle);
  datagram.pad_bytes(4);
  datagram.add_be_float64(_grid_spacing_x);
  datagram.add_be_float64(_grid_spacing_y);
  datagram.add_be_int32(_snap_to_grid);
  datagram.add_be_float64(_grid_size);
  datagram.add_be_int32(_grid_spacing_direction);
  datagram.add_be_int32(_grid_mask);
  datagram.pad_bytes(4);

  return true;
}
