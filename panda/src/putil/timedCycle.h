// Filename: timedCycle.h
// Created by:  jason (01Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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

class EXPCL_PANDA TimedCycle
{
public:
  INLINE TimedCycle();
  INLINE TimedCycle(float cycle_time, int element_count);

  INLINE void set_element_count(int element_count);
  INLINE void set_cycle_time(float cycle_time);
  INLINE int next_element();

public:
  void write_datagram(Datagram &me);
  void fillin(DatagramIterator &scan);

private:
  ClockObject* _global_clock;
  float _cycle_time;
  float _inv_cycle_time;
  double _next_switch;
  int _current_child;
  int _element_count;
};

#include "timedCycle.I"

#endif
