// Filename: qpcollisionHandlerQueue.cxx
// Created by:  drose (16Mar02)
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

#include "qpcollisionHandlerQueue.h"
#include "config_collide.h"

TypeHandle qpCollisionHandlerQueue::_type_handle;

// This class is used in sort_entries(), below.
class qpCollisionEntrySorter {
public:
  qpCollisionEntrySorter(qpCollisionEntry *entry) {
    _entry = entry;
    LVector3f vec =
      entry->get_from_intersection_point() -
      entry->get_from()->get_collision_origin();
    _dist2 = vec.length_squared();
  }
  bool operator < (const qpCollisionEntrySorter &other) const {
    return _dist2 < other._dist2;
  }

  qpCollisionEntry *_entry;
  float _dist2;
};

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerQueue::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
qpCollisionHandlerQueue::
qpCollisionHandlerQueue() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerQueue::begin_group
//       Access: Public, Virtual
//  Description: Will be called by the CollisionTraverser before a new
//               traversal is begun.  It instructs the handler to
//               reset itself in preparation for a number of
//               CollisionEntries to be sent.
////////////////////////////////////////////////////////////////////
void qpCollisionHandlerQueue::
begin_group() {
  _entries.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerQueue::add_entry
//       Access: Public, Virtual
//  Description: Called between a begin_group() .. end_group()
//               sequence for each collision that is detected.
////////////////////////////////////////////////////////////////////
void qpCollisionHandlerQueue::
add_entry(qpCollisionEntry *entry) {
  nassertv(entry != (qpCollisionEntry *)NULL);
  _entries.push_back(entry);
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerQueue::sort_entries
//       Access: Public
//  Description: Sorts all the detected collisions front-to-back by
//               from_intersection_point() so that those intersection
//               points closest to the collider's origin (e.g., the
//               center of the CollisionSphere, or the point_a of a
//               CollisionSegment) appear first.
////////////////////////////////////////////////////////////////////
void qpCollisionHandlerQueue::
sort_entries() {
  // Build up a temporary vector of entries so we can sort the
  // pointers.  This uses the class defined above.
  typedef pvector<qpCollisionEntrySorter> Sorter;
  Sorter sorter;
  sorter.reserve(_entries.size());

  Entries::const_iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    sorter.push_back(qpCollisionEntrySorter(*ei));
  }

  sort(sorter.begin(), sorter.end());
  nassertv(sorter.size() == _entries.size());

  // Now that they're sorted, get them back.  We do this in two steps,
  // building up a temporary vector first, so we don't accidentally
  // delete all the entries when the pointers go away.
  Entries sorted_entries;
  sorted_entries.reserve(sorter.size());
  Sorter::const_iterator si;
  for (si = sorter.begin(); si != sorter.end(); ++si) {
    sorted_entries.push_back((*si)._entry);
  }

  _entries.swap(sorted_entries);
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerQueue::clear_entries
//       Access: Public
//  Description: Removes all the entries from the queue.
////////////////////////////////////////////////////////////////////
void qpCollisionHandlerQueue::
clear_entries() {
  _entries.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerQueue::get_num_entries
//       Access: Public
//  Description: Returns the number of CollisionEntries detected last
//               pass.
////////////////////////////////////////////////////////////////////
int qpCollisionHandlerQueue::
get_num_entries() const {
  return _entries.size();
}

////////////////////////////////////////////////////////////////////
//     Function: qpCollisionHandlerQueue::get_entry
//       Access: Public
//  Description: Returns the nth qpCollisionEntry detected last pass.
////////////////////////////////////////////////////////////////////
qpCollisionEntry *qpCollisionHandlerQueue::
get_entry(int n) const {
  nassertr(n >= 0 && n < (int)_entries.size(), NULL);
  return _entries[n];
}
