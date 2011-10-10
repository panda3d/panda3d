// Filename: timedCycle.cxx
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
#include "pandabase.h"
#include "timedCycle.h"

#include "datagram.h"
#include "datagramIterator.h"

////////////////////////////////////////////////////////////////////
//     Function: TimedCycle::write_object
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void TimedCycle::
write_datagram(Datagram &me) {
  me.add_stdfloat(_cycle_time);
  me.add_uint16(_element_count);
}

////////////////////////////////////////////////////////////////////
//     Function: TimedCycle::fillin
//       Access: Protected
//  Description: This internal function is called by make_TimedCycle to
//               read in all of the relevant data from the BamFile for
//               the new TimedCycle.
////////////////////////////////////////////////////////////////////
void TimedCycle::
fillin(DatagramIterator &scan) {
  _cycle_time = scan.get_stdfloat();
  _element_count = scan.get_uint16();
  _inv_cycle_time = 1. / _cycle_time;

  _global_clock = ClockObject::get_global_clock();
  _next_switch = _global_clock->get_real_time() + _cycle_time;
  _current_child = 0;
}
