// Filename: pStatViewLevel.I
// Created by:  drose (19Jul00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: PStatViewLevel::get_collector
//       Access: Public
//  Description: Returns the Collector index associated with this
//               level.
////////////////////////////////////////////////////////////////////
INLINE int PStatViewLevel::
get_collector() const {
  return _collector;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatViewLevel::get_time_alone
//       Access: Public
//  Description: Returns the total elapsed time spent by this
//               Collector, not including any time spent in its child
//               Collectors.
////////////////////////////////////////////////////////////////////
INLINE double PStatViewLevel::
get_time_alone() const {
  return _time_alone;
}
