// Filename: pStatFrameData.cxx
// Created by:  drose (10Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pStatFrameData.h"
#include "pStatClientVersion.h"

#include "datagram.h"
#include "datagramIterator.h"

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::write_datagram
//       Access: Public
//  Description: Writes the definition of the FrameData to the
//               datagram.
////////////////////////////////////////////////////////////////////
void PStatFrameData::
write_datagram(Datagram &destination) const {
  Data::const_iterator di;
  destination.add_uint16(_time_data.size());
  for (di = _time_data.begin(); di != _time_data.end(); ++di) {
    destination.add_uint16((*di)._index);
    destination.add_float32((*di)._value);
  }
  destination.add_uint16(_level_data.size());
  for (di = _level_data.begin(); di != _level_data.end(); ++di) {
    destination.add_uint16((*di)._index);
    destination.add_float32((*di)._value);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::read_datagram
//       Access: Public
//  Description: Extracts the FrameData definition from the datagram.
////////////////////////////////////////////////////////////////////
void PStatFrameData::
read_datagram(DatagramIterator &source, PStatClientVersion *) {
  clear();

  int i;
  int time_size = source.get_uint16();
  for (i = 0; i < time_size; i++) {
    nassertv(source.get_remaining_size() > 0);
    DataPoint dp;
    dp._index = source.get_uint16();
    dp._value = source.get_float32();
    _time_data.push_back(dp);
  }
  int level_size = source.get_uint16();
  for (i = 0; i < level_size; i++) {
    nassertv(source.get_remaining_size() > 0);
    DataPoint dp;
    dp._index = source.get_uint16();
    dp._value = source.get_float32();
    _level_data.push_back(dp);
  }
  nassertv(source.get_remaining_size() == 0);
}

