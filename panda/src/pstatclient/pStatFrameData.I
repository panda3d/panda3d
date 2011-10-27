// Filename: pStatFrameData.I
// Created by:  drose (10Jul00)
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


////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::is_time_empty
//       Access: Public
//  Description: Returns true if there are no time events in the frame
//               data, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool PStatFrameData::
is_time_empty() const {
  return _time_data.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::is_level_empty
//       Access: Public
//  Description: Returns true if there are no levels indicated in the
//               frame data, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool PStatFrameData::
is_level_empty() const {
  return _level_data.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::is_empty
//       Access: Public
//  Description: Returns true if the FrameData has no time or level
//               data.
////////////////////////////////////////////////////////////////////
INLINE bool PStatFrameData::
is_empty() const {
  return is_time_empty() && is_level_empty();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::clear
//       Access: Public
//  Description: Removes all the data points from the frame data, in
//               preparation for building up a new frame's worth.
////////////////////////////////////////////////////////////////////
INLINE void PStatFrameData::
clear() {
  _time_data.clear();
  _level_data.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::swap
//       Access: Public
//  Description: Exchanges the data in this object with the data in
//               the other.
////////////////////////////////////////////////////////////////////
INLINE void PStatFrameData::
swap(PStatFrameData &other) {
  _time_data.swap(other._time_data);
  _level_data.swap(other._level_data);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::add_start
//       Access: Public
//  Description: Adds a 'start collector' data point to the frame
//               data.
////////////////////////////////////////////////////////////////////
INLINE void PStatFrameData::
add_start(int index, double time) {
#ifdef _DEBUG
  nassertv((index & 0x7fff) == index);
#endif
  DataPoint dp;
  dp._index = index;
  dp._value = time;
  _time_data.push_back(dp);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::add_stop
//       Access: Public
//  Description: Adds a 'stop collector' data point to the frame
//               data.
////////////////////////////////////////////////////////////////////
INLINE void PStatFrameData::
add_stop(int index, double time) {
#ifdef _DEBUG
  nassertv((index & 0x7fff) == index);
#endif
  DataPoint dp;
  dp._index = index | 0x8000;
  dp._value = time;
  _time_data.push_back(dp);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::add_level
//       Access: Public
//  Description: Adds a particular level value associated with a given
//               collector to the frame data.
////////////////////////////////////////////////////////////////////
INLINE void PStatFrameData::
add_level(int index, double level) {
#ifdef _DEBUG
  nassertv((index & 0xffff) == index);
#endif
  DataPoint dp;
  dp._index = index;
  dp._value = level;
  _level_data.push_back(dp);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::get_start
//       Access: Public
//  Description: Returns the time of the first data point in the frame
//               data.  This will generally be the time of the start
//               of the frame.
////////////////////////////////////////////////////////////////////
INLINE double PStatFrameData::
get_start() const {
  if (is_empty()) {
    return 0.0;
  }

  return _time_data.front()._value;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::get_end
//       Access: Public
//  Description: Returns the time of the last data point in the frame
//               data.  This will generally be the time of the end
//               of the frame.
////////////////////////////////////////////////////////////////////
INLINE double PStatFrameData::
get_end() const {
  nassertr(!is_empty(), 0.0);

  return _time_data.back()._value;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::get_net_time
//       Access: Public
//  Description: Returns the total time elapsed for the frame.
////////////////////////////////////////////////////////////////////
INLINE double PStatFrameData::
get_net_time() const {
  nassertr(!is_empty(), 0.0);

  return _time_data.back()._value - _time_data.front()._value;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::get_num_events
//       Access: Public
//  Description: Returns the number of individual events stored in the
//               FrameData.
////////////////////////////////////////////////////////////////////
INLINE int PStatFrameData::
get_num_events() const {
  return _time_data.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::get_time_collector
//       Access: Public
//  Description: Returns the index of the collector associated with
//               the nth event.
////////////////////////////////////////////////////////////////////
INLINE int PStatFrameData::
get_time_collector(int n) const {
  nassertr(n >= 0 && n < (int)_time_data.size(), 0);
  return _time_data[n]._index & 0x7fff;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::is_start
//       Access: Public
//  Description: Returns true if the nth event represents a start
//               event, or false if it represents a stop event.
////////////////////////////////////////////////////////////////////
INLINE bool PStatFrameData::
is_start(int n) const {
  nassertr(n >= 0 && n < (int)_time_data.size(), 0);
  return (_time_data[n]._index & 0x8000) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::get_time
//       Access: Public
//  Description: Returns the timestamp of the nth event, in seconds
//               elapsed since some undefined epoch (which is
//               guaranteed to be shared among all events returned
//               from a given client).
////////////////////////////////////////////////////////////////////
INLINE double PStatFrameData::
get_time(int n) const {
  nassertr(n >= 0 && n < (int)_time_data.size(), 0);
  return _time_data[n]._value;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::get_num_levels
//       Access: Public
//  Description: Returns the number of individual level values stored
//               in the FrameData.
////////////////////////////////////////////////////////////////////
INLINE int PStatFrameData::
get_num_levels() const {
  return _level_data.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::get_level_collector
//       Access: Public
//  Description: Returns the index of the collector associated with
//               the nth level value.
////////////////////////////////////////////////////////////////////
INLINE int PStatFrameData::
get_level_collector(int n) const {
  nassertr(n >= 0 && n < (int)_level_data.size(), 0);
  return _level_data[n]._index;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::get_level
//       Access: Public
//  Description: Returns the height of the nth level value.
////////////////////////////////////////////////////////////////////
INLINE double PStatFrameData::
get_level(int n) const {
  nassertr(n >= 0 && n < (int)_level_data.size(), 0);
  return _level_data[n]._value;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::DataPoint::operator <
//       Access: Public
//  Description: Orders the data points by time.
////////////////////////////////////////////////////////////////////
INLINE bool PStatFrameData::DataPoint::
operator < (const PStatFrameData::DataPoint &other) const {
  return _value < other._value;
}

