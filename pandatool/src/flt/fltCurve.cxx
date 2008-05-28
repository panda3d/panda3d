// Filename: fltCurve.cxx
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

#include "fltCurve.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"
#include "fltHeader.h"
#include "fltMaterial.h"

TypeHandle FltCurve::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltCurve::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltCurve::
FltCurve(FltHeader *header) : FltBeadID(header) {
  _curve_type = CT_b_spline;
}

////////////////////////////////////////////////////////////////////
//     Function: FltCurve::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this bead based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltCurve::
extract_record(FltRecordReader &reader) {
  if (!FltBeadID::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_curve, false);
  DatagramIterator &iterator = reader.get_iterator();

  iterator.skip_bytes(4);
  _curve_type = (CurveType)iterator.get_be_int32();

  int num_control_points = iterator.get_be_int32();
  iterator.skip_bytes(8);
  for (int i = 0; i < num_control_points; i++) {
    double x = iterator.get_be_float64();
    double y = iterator.get_be_float64();
    double z = iterator.get_be_float64();
    _control_points.push_back(LPoint3d(x, y, z));
  }

  check_remaining_size(iterator);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltCurve::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltCurve::
build_record(FltRecordWriter &writer) const {
  if (!FltBeadID::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_curve);
  Datagram &datagram = writer.update_datagram();

  datagram.pad_bytes(4);
  datagram.add_be_int32(_curve_type);
  datagram.add_be_int32(_control_points.size());
  datagram.pad_bytes(8);

  ControlPoints::const_iterator ci;
  for (ci = _control_points.begin(); ci != _control_points.end(); ++ci) {
    const LPoint3d &p = (*ci);
    datagram.add_be_float64(p[0]);
    datagram.add_be_float64(p[1]);
    datagram.add_be_float64(p[2]);
  }

  return true;
}
