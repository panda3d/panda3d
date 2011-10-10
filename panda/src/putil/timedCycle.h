// Filename: timedCycle.h
// Created by:  jason (01Aug00)
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

#ifndef TIMED_CYCLE_H
#define TIMED_CYCLE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "pandabase.h"

#include "clockObject.h"

class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
//       Class : TimedCycle
// Description : A class for anything that needs to cycle over
//               some finite list of elements in increments based on
//               time.  All time variables are assumed to be set in
//               seconds.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA_PUTIL TimedCycle
{
public:
  INLINE TimedCycle();
  INLINE TimedCycle(PN_stdfloat cycle_time, int element_count);

  INLINE void set_element_count(int element_count);
  INLINE void set_cycle_time(PN_stdfloat cycle_time);
  INLINE int next_element();

public:
  void write_datagram(Datagram &me);
  void fillin(DatagramIterator &scan);

private:
  ClockObject* _global_clock;
  PN_stdfloat _cycle_time;
  PN_stdfloat _inv_cycle_time;
  double _next_switch;
  int _current_child;
  int _element_count;
};

#include "timedCycle.I"

#endif
