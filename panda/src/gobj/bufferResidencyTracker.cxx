// Filename: bufferResidencyTracker.cxx
// Created by:  drose (16Mar06)
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

#include "bufferResidencyTracker.h"
#include "bufferContext.h"
#include "clockObject.h"

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
//     Function: BufferResidencyTracker::begin_frame
//       Access: Public
//  Description: To be called at the beginning of a frame, this
//               initializes the active/inactive status.
////////////////////////////////////////////////////////////////////
void BufferResidencyTracker::
begin_frame() {
#ifdef DO_PSTATS
  int this_frame = ClockObject::get_global_clock()->get_frame_count();
  if (_active_frame != this_frame) {
    _active_frame = this_frame;

    // Move all of the previously "active" objects into "inactive".
    // They'll get re-added to "active" as they get rendered.
    move_inactive(_chains[S_inactive_nonresident],
                  _chains[S_active_nonresident]);
    move_inactive(_chains[S_inactive_resident],
                  _chains[S_active_resident]);
  }
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: BufferResidencyTracker::end_frame
//       Access: Public
//  Description: To be called at the end of a frame, this
//               updates the PStatCollectors appropriately.
////////////////////////////////////////////////////////////////////
void BufferResidencyTracker::
end_frame() {
  _inactive_nonresident_collector.set_level(_chains[S_inactive_nonresident].get_total_size());
  _active_nonresident_collector.set_level(_chains[S_active_nonresident].get_total_size());
  _inactive_resident_collector.set_level(_chains[S_inactive_resident].get_total_size());
  _active_resident_collector.set_level(_chains[S_active_resident].get_total_size());
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
