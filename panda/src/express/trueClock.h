// Filename: trueClock.h
// Created by:  drose (04Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef TRUECLOCK_H
#define TRUECLOCK_H

#include "pandabase.h"
#include "typedef.h"

////////////////////////////////////////////////////////////////////
//       Class : TrueClock
// Description : An interface to whatever real-time clock we might
//               have available in the current environment.  There is
//               only one TrueClock in existence, and it constructs
//               itself.
//
//               The TrueClock returns elapsed real time in seconds
//               since some undefined epoch.  Since it is not defined
//               at what time precisely the clock indicates zero, this
//               value can only be meaningfully used to measure
//               elapsed time, by sampling it at two different times
//               and subtracting.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS TrueClock {
public:
  INLINE static TrueClock *get_ptr();

  // get_long_time() returns the most accurate timer we have over a
  // long interval.  It may not be very precise for measuring short
  // intervals, but it should not drift substantially over the long
  // haul.
  double get_long_time() const;

  // get_short_time() returns the most precise timer we have over a
  // short interval.  It may tend to drift over the long haul, but it
  // should have lots of digits to measure short intervals very
  // precisely.
  double get_short_time() const;

protected:
  TrueClock();
  INLINE ~TrueClock();

  static TrueClock *_global_ptr;
};

void get_true_time_of_day(ulong &sec, ulong &usec);

#include "trueClock.I"

#endif
