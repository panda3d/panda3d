// Filename: profileTimer.I
// Created by:  
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

INLINE void ProfileTimer::
on() {
  _on = TrueClock::get_global_ptr()->get_short_time();
}


INLINE double ProfileTimer::
getTime() {
  double time = TrueClock::get_global_ptr()->get_short_time();
  double et=_elapsedTime+=time-_on;
  _on=time;
  _elapsedTime=0.0;
  return et;
}


INLINE void ProfileTimer::
mark(const char* tag) {
  if (!_entries) {
    cerr << "ProfileTimer::mark !_entries" << endl;
    exit(1);
  }
  if (_entryCount < _maxEntries-1) {
    TimerEntry& p=_entries[_entryCount];
    p._tag=tag;
    p._time=getTime();
    ++_entryCount;
  } else {
    _entries[_entryCount]._tag="*** Overflow ***";
  }
}


INLINE void ProfileTimer::
off() {
  double time = TrueClock::get_global_ptr()->get_short_time();
  _elapsedTime+=time-_on;
}


INLINE void ProfileTimer::
off(const char* tag) {
  double time = TrueClock::get_global_ptr()->get_short_time();
  _elapsedTime+=time-_on;
  mark(tag);
}


INLINE ProfileTimer::AutoTimer::
~AutoTimer() {
  // If the AutoTimer is the first auto ctor, then it will
  // be the last auto dtor, for that block.  Therefore, now
  // is the time to mark the time for the block/function:
  _profile.mark(_tag);
  --_profile._autoTimerCount;
}
