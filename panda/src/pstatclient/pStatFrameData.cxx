// Filename: pStatFrameData.cxx
// Created by:  drose (10Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "pStatFrameData.h"

#include <datagram.h>
#include <datagramIterator.h>

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::write_datagram
//       Access: Public
//  Description: Writes the definition of the FrameData to the
//               datagram.
////////////////////////////////////////////////////////////////////
void PStatFrameData::
write_datagram(Datagram &destination) const {
  Data::const_iterator di;
  for (di = _data.begin(); di != _data.end(); ++di) {
    destination.add_uint16((*di)._index);
    destination.add_float64((*di)._time);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::read_datagram
//       Access: Public
//  Description: Extracts the FrameData definition from the datagram.
////////////////////////////////////////////////////////////////////
void PStatFrameData::
read_datagram(DatagramIterator &source) {
  clear();
  while (source.get_remaining_size() > 0) {
    DataPoint dp;
    dp._index = source.get_uint16();
    dp._time = source.get_float64();
    _data.push_back(dp);
  }
}

