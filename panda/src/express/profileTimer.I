// ProfileTimer.I

INLINE void ProfileTimer::
on() {
  _on = TrueClock::get_ptr()->get_real_time();
}


INLINE double ProfileTimer::
getTime() {
  double time = TrueClock::get_ptr()->get_real_time();
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
  double time = TrueClock::get_ptr()->get_real_time();
  _elapsedTime+=time-_on;
}


INLINE void ProfileTimer::
off(const char* tag) {
  double time = TrueClock::get_ptr()->get_real_time();
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
