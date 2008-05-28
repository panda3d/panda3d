// Filename: profileTimer.cxx
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

#include "profileTimer.h"

#include "pmap.h"

using namespace std;

// See ProfileTimer.h for documentation.


EXPCL_PANDAEXPRESS ProfileTimer Skyler_timer_global=ProfileTimer("startup");

ProfileTimer* ProfileTimer::_head;

ProfileTimer::
ProfileTimer(const char* name, int maxEntries) :
  _entries(0),
  _autoTimerCount(0) {
  // Keep a list of the ProfileTimers, so we can print them:
  _next=_head;
  _head=this;
  if (name) {
    init(name, maxEntries);
  }
}

ProfileTimer::
ProfileTimer(const ProfileTimer& other) {
  // Add to list:
  _next=_head;
  _head=this;
  // init it:
  _name=other._name;
  _maxEntries=other._maxEntries;
  if (_name) {
    init(_name, _maxEntries);
  }
  // Copy other entries:
  _on=other._on;
  _elapsedTime=other._elapsedTime;
  _autoTimerCount=other._autoTimerCount;
  _entryCount=other._entryCount;
  if (other._entries) {
    memcpy(_entries, other._entries, _entryCount * sizeof(TimerEntry));
  }
}

ProfileTimer::
~ProfileTimer() {
  PANDA_FREE_ARRAY(_entries);
  // Remove this from the list:
  if (_head==this) {
    _head=_next;
  } else {
    ProfileTimer* p=_head;
    ProfileTimer* prior=p;
    while (p) {
      if (p==this) {
        prior->_next=_next;
        break;
      }
      prior=p;
      p=p->_next;
    }
  }
}

void ProfileTimer::
init(const char* name, int maxEntries) {
  _name=name;
  _maxEntries=maxEntries;
  _entries = (TimerEntry *)PANDA_MALLOC_ARRAY(_maxEntries * sizeof(TimerEntry));
  _entryCount=0;
  _elapsedTime=0.0;
  _on=0.0;
}

double ProfileTimer::
getTotalTime() const {
  double total=0;
  int i;
  for (i=0; i<_entryCount; ++i) {
    TimerEntry& te=_entries[i];
    total+=te._time;
  }
  return total;
}

void ProfileTimer::
consolidateAllTo(ostream &out) {
  ProfileTimer* p=_head;
  while (p) {
    p->consolidateTo(out);
    p=p->_next;
  }
}

void ProfileTimer::
consolidateTo(ostream &out) const {
  pmap<string, double> entries;
  int i;
  for (i=0; i<_entryCount; ++i) {
    TimerEntry& te=_entries[i];
    entries[te._tag]+=te._time;
  }
  out << "-------------------------------------------------------------------\n"
    << "Profile Timing of " << _name
    << "\n\n"; // ...should print data and time too.
  double total=0;
  {
  pmap<string, double>::const_iterator i=entries.begin();
  for (;i!=entries.end(); ++i) {
    out << "  " << setw(50) << i->first << ": "
    << setiosflags(ios::fixed) << setprecision(6) << setw(10) << i->second << "\n";
    total+=i->second;
  }
  }
  out << "\n                       [Total Time: "
    << setiosflags(ios::fixed) << setprecision(6) << total
    << " seconds]\n"
    << "-------------------------------------------------------------------\n";
  out << endl;
}

void ProfileTimer::
printAllTo(ostream &out) {
  ProfileTimer* p=_head;
  while (p) {
    p->printTo(out);
    p=p->_next;
  }
}

void ProfileTimer::
printTo(ostream &out) const {
  out << "-------------------------------------------------------------------\n"
    << "Profile Timing of " << _name
    << "\n\n"; // ...should print data and time too.
  double total=0;
  int i;
  for (i=0; i<_entryCount; ++i) {
    TimerEntry& te=_entries[i];
    out << "  " << setw(50) << te._tag << ": "
    << setiosflags(ios::fixed) << setprecision(6) << setw(10) << te._time << "\n";
    total+=te._time;
  }
  out << "\n                       [Total Time: "
    << setiosflags(ios::fixed) << setprecision(6) << total
    << " seconds]\n"
    << "-------------------------------------------------------------------\n";
  out << endl;
}

ProfileTimer::AutoTimer::AutoTimer(ProfileTimer& profile, const char* tag) :
    _profile(profile) {
  _tag=tag;
  if (_profile._autoTimerCount) {
    // ...this is a nested call to another AutoTimer.
    // Assign the time to the prior AutoTimer:
    _profile.mark(_profile._entries[_profile._entryCount-1]._tag);
  } else {
    // ...this is not a nested call.
    _profile.mark("other");
  }
  // Tell the profile that it's in an AutoTimer:
  ++_profile._autoTimerCount;
  _profile.mark(_tag);
}



