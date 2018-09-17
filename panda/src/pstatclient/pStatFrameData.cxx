/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatFrameData.cxx
 * @author drose
 * @date 2000-07-10
 */

#include "pStatFrameData.h"
#include "pStatClientVersion.h"
#include "config_pstatclient.h"

#include "datagram.h"
#include "datagramIterator.h"

#include <algorithm>

/**
 * Ensures the frame data is in monotonically increasing order by time.
 */
void PStatFrameData::
sort_time() {
  std::stable_sort(_time_data.begin(), _time_data.end());
}

/**
 * Writes the definition of the FrameData to the datagram.  Returns true on
 * success, false on failure.
 */
bool PStatFrameData::
write_datagram(Datagram &destination, PStatClient *client) const {
  Data::const_iterator di;
  if (_time_data.size() >= 65536 || _level_data.size() >= 65536) {
    pstats_cat.info()
      << "Dropping frame with " << _time_data.size()
      << " time measurements and " << _level_data.size()
      << " level measurements.\n";
    return false;
  }

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

  return true;
}

/**
 * Extracts the FrameData definition from the datagram.
 */
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
