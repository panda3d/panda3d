// Filename: pStatFrameData.I
// Created by:  drose (10Jul00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::is_empty
//       Access: Public
//  Description: Returns true if there are no data points in the frame
//               data, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool PStatFrameData::
is_empty() const {
  return _data.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::clear
//       Access: Public
//  Description: Removes all the data points from the frame data, in
//               preparation for building up a new frame's worth.
////////////////////////////////////////////////////////////////////
INLINE void PStatFrameData::
clear() {
  _data.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::add_start
//       Access: Public
//  Description: Adds a 'start collector' data point to the frame
//               data.
////////////////////////////////////////////////////////////////////
INLINE void PStatFrameData::
add_start(int index, double time) {
  DataPoint dp;
  dp._index = index;
  dp._time = time;
  _data.push_back(dp);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::add_stop
//       Access: Public
//  Description: Adds a 'stop collector' data point to the frame
//               data.
////////////////////////////////////////////////////////////////////
INLINE void PStatFrameData::
add_stop(int index, double time) {
  DataPoint dp;
  dp._index = index | 0x8000;
  dp._time = time;
  _data.push_back(dp);
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
  nassertr(!is_empty(), 0.0);

  return _data.front()._time;
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

  return _data.back()._time;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::get_net_time
//       Access: Public
//  Description: Returns the total time elapsed for the frame.
////////////////////////////////////////////////////////////////////
INLINE double PStatFrameData::
get_net_time() const {
  nassertr(!is_empty(), 0.0);

  return _data.back()._time - _data.front()._time;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::get_num_events
//       Access: Public
//  Description: Returns the number of individual events stored in the
//               FrameData.
////////////////////////////////////////////////////////////////////
INLINE int PStatFrameData::
get_num_events() const {
  return _data.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatFrameData::get_collector
//       Access: Public
//  Description: Returns the index of the collector associated with
//               the nth event.  If this is the first time the
//               collector appears in the event list, it indicates the
//               collector has started; if it is the second time, it
//               indicates the collector has stopped.  Similarly for
//               repeated appearances of the same collector.
////////////////////////////////////////////////////////////////////
INLINE int PStatFrameData::
get_collector(int n) const {
  nassertr(n >= 0 && n < (int)_data.size(), 0);
  return _data[n]._index;
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
  nassertr(n >= 0 && n < (int)_data.size(), 0);
  return _data[n]._time;
}
