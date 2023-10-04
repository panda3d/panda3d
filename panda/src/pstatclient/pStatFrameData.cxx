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
  if (_time_data.size() >= 65536 || _level_data.size() >= 65536) {
    pstats_cat.info()
      << "Dropping frame with " << _time_data.size()
      << " time measurements and " << _level_data.size()
      << " level measurements.\n";
    return false;
  }

#if !defined(WORDS_BIGENDIAN) || defined(__GNUC__)
  // Hand-roll this, significantly more efficient for many data points
  size_t size = (_time_data.size() + _level_data.size()) * 6 + 8;
  PTA_uchar array = destination.modify_array();
  size_t offset = array.size();
  array.resize(offset + size);
  unsigned char *data = &array[0] + offset;

  uint16_t *ptr = (uint16_t *)data;

#ifdef WORDS_BIGENDIAN
  *(uint32_t *)ptr = __builtin_bswap32(_time_data.size());
  ptr += 2;

  for (const DataPoint &dp : _time_data) {
    *ptr++ = __builtin_bswap16(dp._index);
    PN_float32 v = (PN_float32)dp._value;
    *(uint32_t *)ptr = __builtin_bswap32(reinterpret_cast<uint32_t &>(v));
    ptr += 2;
  }

  *(uint32_t *)ptr = __builtin_bswap16(_level_data.size());
  ptr += 2;

  for (const DataPoint &dp : _level_data) {
    *ptr++ = __builtin_bswap16(dp._index);
    PN_float32 v = (PN_float32)dp._value;
    *(uint32_t *)ptr = __builtin_bswap32(reinterpret_cast<uint32_t &>(v));
    ptr += 2;
  }
#else
  *(uint32_t *)ptr = _time_data.size();
  ptr += 2;

  for (const DataPoint &dp : _time_data) {
    *ptr++ = dp._index;
    *(PN_float32 *)ptr = dp._value;
    ptr += 2;
  }

  *(uint32_t *)ptr = _level_data.size();
  ptr += 2;

  for (const DataPoint &dp : _level_data) {
    *ptr++ = dp._index;
    *(PN_float32 *)ptr = dp._value;
    ptr += 2;
  }
#endif

#else
  destination.add_uint32(_time_data.size());
  for (const DataPoint &dp : _time_data) {
    destination.add_uint16(dp._index);
    destination.add_float32(dp._value);
  }
  destination.add_uint32(_level_data.size());
  for (const DataPoint &dp : _level_data) {
    destination.add_uint16(dp._index);
    destination.add_float32(dp._value);
  }
#endif

  return true;
}

/**
 * Extracts the FrameData definition from the datagram.
 */
void PStatFrameData::
read_datagram(DatagramIterator &source, PStatClientVersion *version) {
  clear();

  {
    size_t time_size;
    if (version->is_at_least(3, 2)) {
      time_size = source.get_uint32();
    } else {
      time_size = source.get_uint16();
    }
    _time_data.resize(time_size);
    for (DataPoint &dp : _time_data) {
      nassertv(source.get_remaining_size() > 0);
      dp._index = source.get_uint16();
      dp._value = source.get_float32();
    }
  }

  {
    size_t level_size;
    if (version->is_at_least(3, 2)) {
      level_size = source.get_uint32();
    } else {
      level_size = source.get_uint16();
    }
    _level_data.resize(level_size);
    for (DataPoint &dp : _level_data) {
      nassertv(source.get_remaining_size() > 0);
      dp._index = source.get_uint16();
      dp._value = source.get_float32();
    }
  }

  //nassertv(source.get_remaining_size() == 0);
}
