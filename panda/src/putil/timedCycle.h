// Filename: timedCycle.h
// Created by:  jason (01Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TIMED_CYCLE_H
#define TIMED_CYCLE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

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

#include <timedCycle.I>

#endif
