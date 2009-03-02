// Filename: bufferResidencyTracker.cxx
// Created by:  drose (16Mar06)
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

#include "bufferResidencyTracker.h"
#include "bufferContext.h"
#include "clockObject.h"
#include "indent.h"

PStatCollector BufferResidencyTracker::_gmem_collector("Graphics memory");

////////////////////////////////////////////////////////////////////
//     Function: BufferResidencyTracker::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
BufferResidencyTracker::
BufferResidencyTracker(const string &pgo_name, const string &type_name) :
  _pgo_collector(_gmem_collector, pgo_name),
  _active_resident_collector(PStatCollector(_pgo_collector, "Active"), type_name),
  _active_nonresident_collector(PStatCollector(_pgo_collector, "Thrashing"), type_name),
  _inactive_resident_collector(PStatCollector(_pgo_collector, "Inactive"), type_name),
  _inactive_nonresident_collector(PStatCollector(_pgo_collector, "Nonresident"), type_name),
  _active_frame(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BufferResidencyTracker::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
BufferResidencyTracker::
~BufferResidencyTracker() {
  _inactive_nonresident_collector.set_level(0);
  _active_nonresident_collector.set_level(0);
  _inactive_resident_collector.set_level(0);
  _active_resident_collector.set_level(0);
}

////////////////////////////////////////////////////////////////////
//     Function: BufferResidencyTracker::begin_frame
//       Access: Public
//  Description: To be called at the beginning of a frame, this
//               initializes the active/inactive status.
////////////////////////////////////////////////////////////////////
void BufferResidencyTracker::
begin_frame(Thread *current_thread) {
  int this_frame = ClockObject::get_global_clock()->get_frame_count(current_thread);
  if (_active_frame != this_frame) {
    _active_frame = this_frame;

    // Move all of the previously "active" objects into "inactive".
    // They'll get re-added to "active" as they get rendered.
    move_inactive(_chains[S_inactive_nonresident],
                  _chains[S_active_nonresident]);
    move_inactive(_chains[S_inactive_resident],
                  _chains[S_active_resident]);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BufferResidencyTracker::end_frame
//       Access: Public
//  Description: To be called at the end of a frame, this
//               updates the PStatCollectors appropriately.
////////////////////////////////////////////////////////////////////
void BufferResidencyTracker::
end_frame(Thread *current_thread) {
  _inactive_nonresident_collector.set_level(_chains[S_inactive_nonresident].get_total_size());
  _active_nonresident_collector.set_level(_chains[S_active_nonresident].get_total_size());
  _inactive_resident_collector.set_level(_chains[S_inactive_resident].get_total_size());
  _active_resident_collector.set_level(_chains[S_active_resident].get_total_size());
}

////////////////////////////////////////////////////////////////////
//     Function: BufferResidencyTracker::set_levels
//       Access: Public
//  Description: Resets the pstats levels to their appropriate values,
//               possibly in the middle of a frame.
////////////////////////////////////////////////////////////////////
void BufferResidencyTracker::
set_levels() {
  _inactive_nonresident_collector.set_level(_chains[S_inactive_nonresident].get_total_size());
  _active_nonresident_collector.set_level(_chains[S_active_nonresident].get_total_size());
  _inactive_resident_collector.set_level(_chains[S_inactive_resident].get_total_size());
  _active_resident_collector.set_level(_chains[S_active_resident].get_total_size());
}

////////////////////////////////////////////////////////////////////
//     Function: BufferResidencyTracker::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void BufferResidencyTracker::
write(ostream &out, int indent_level) const {
  if (_chains[S_inactive_nonresident].get_count() != 0) {
    indent(out, indent_level) << "Inactive nonresident:\n";
    _chains[S_inactive_nonresident].write(out, indent_level + 2);
  }

  if (_chains[S_active_nonresident].get_count() != 0) {
    indent(out, indent_level) << "Active nonresident:\n";
    _chains[S_active_nonresident].write(out, indent_level + 2);
  }

  if (_chains[S_inactive_resident].get_count() != 0) {
    indent(out, indent_level) << "Inactive resident:\n";
    _chains[S_inactive_resident].write(out, indent_level + 2);
  }

  if (_chains[S_active_resident].get_count() != 0) {
    indent(out, indent_level) << "Active resident:\n";
    _chains[S_active_resident].write(out, indent_level + 2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BufferResidencyTracker::move_inactive
//       Access: Private
//  Description: Moves all of the "active" objects into "inactive".
////////////////////////////////////////////////////////////////////
void BufferResidencyTracker::
move_inactive(BufferContextChain &inactive, BufferContextChain &active) {
  BufferContext *node = active.get_first();
  while (node != (BufferContext *)NULL) {
    nassertv((node->_residency_state & S_active) != 0);
    node->_residency_state &= ~S_active;
    node = node->get_next();
  }

  inactive.take_from(active);
}
