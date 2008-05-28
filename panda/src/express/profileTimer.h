// Filename: profileTimer.h
// Created by: skyler 
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
#ifndef PROFILETIMER_H //[
#define PROFILETIMER_H

#include "pandabase.h"
#include "trueClock.h"

/*
    ProfileTimer

    HowTo:
      Create a ProfileTimer and hold onto it.
      Call init() whenever you like (the timer doesn't
        start yet).
      Call on() to start the timer.
      While the timer is on, call mark() at each point of interest,
        in the code you are timing.
      You can turn the timer off() and on() to skip things you
        don't want to time.
      When your timing is finished, call printTo() to see the
        results (e.g. myTimer.printTo(cerr)).

    Notes:
      You should be able to time things down to the millisecond
      well enough, but if you call on() and off() within micro-
      seconds of each other, I don't think you'll get very good
      results.
*/
class EXPCL_PANDAEXPRESS ProfileTimer {
  enum { MaxEntriesDefault=4096 };
PUBLISHED:
  ProfileTimer(const char* name=0, int maxEntries=MaxEntriesDefault);
  ProfileTimer(const ProfileTimer& other);
  ~ProfileTimer();

  void init(const char* name, int maxEntries=MaxEntriesDefault);

  void on();
  void mark(const char* tag);
  void off();
  void off(const char* tag);

  // Don't call any of the following during timing:
  // (Because they are slow, not because anything will break).
  double getTotalTime() const;
  static void consolidateAllTo(ostream &out=cout);
  void consolidateTo(ostream &out=cout) const;
  static void printAllTo(ostream &out=cout);
  void printTo(ostream &out=cout) const;

public:
  /*
      e.g.
      void Foo() {
        ProfileTimer::AutoTimer(myProfiler, "Foo()");
        ...
      }
  */
  class EXPCL_PANDAEXPRESS AutoTimer {
  public:
    AutoTimer(ProfileTimer& profile, const char* tag);
    ~AutoTimer();

  protected:
    ProfileTimer& _profile;
    const char* _tag;
  };

protected:
  static ProfileTimer* _head;
  ProfileTimer* _next;
  class TimerEntry {
  public:
    const char* _tag; // not owned by this.
    double _time;
  };
  double _on;
  double _elapsedTime;
  const char* _name; // not owned by this.
  int _maxEntries;
  int _entryCount;
  TimerEntry* _entries;
  int _autoTimerCount; // see class AutoTimer

  double getTime();

  friend class ProfileTimer::AutoTimer;
};

#include "profileTimer.I"

#endif //]
