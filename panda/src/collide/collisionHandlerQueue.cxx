/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerQueue.cxx
 * @author drose
 * @date 2002-03-16
 */

#include "collisionHandlerQueue.h"
#include "config_collide.h"
#include "indent.h"

TypeHandle CollisionHandlerQueue::_type_handle;

// This class is used in sort_entries(), below.
class CollisionEntrySorter {
public:
  CollisionEntrySorter(CollisionEntry *entry) {
    _entry = entry;
    if (entry->has_surface_point()) {
      LVector3 vec =
        entry->get_surface_point(entry->get_from_node_path()) -
        entry->get_from()->get_collision_origin();
      _dist2 = vec.length_squared();
    }
    else {
      _dist2 = make_inf((PN_stdfloat)0);
    }
  }
  bool operator < (const CollisionEntrySorter &other) const {
    return _dist2 < other._dist2;
  }

  CollisionEntry *_entry;
  PN_stdfloat _dist2;
};

/**
 *
 */
CollisionHandlerQueue::
CollisionHandlerQueue() {
}

/**
 * Will be called by the CollisionTraverser before a new traversal is begun.
 * It instructs the handler to reset itself in preparation for a number of
 * CollisionEntries to be sent.
 */
void CollisionHandlerQueue::
begin_group() {
  _entries.clear();
}

/**
 * Called between a begin_group() .. end_group() sequence for each collision
 * that is detected.
 */
void CollisionHandlerQueue::
add_entry(CollisionEntry *entry) {
  nassertv(entry != nullptr);
  _entries.push_back(entry);
}

/**
 * Sorts all the detected collisions front-to-back by
 * from_intersection_point() so that those intersection points closest to the
 * collider's origin (e.g., the center of the CollisionSphere, or the point_a
 * of a CollisionSegment) appear first.
 */
void CollisionHandlerQueue::
sort_entries() {
  // Build up a temporary vector of entries so we can sort the pointers.  This
  // uses the class defined above.
  typedef pvector<CollisionEntrySorter> Sorter;
  Sorter sorter;
  sorter.reserve(_entries.size());

  Entries::const_iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    sorter.push_back(CollisionEntrySorter(*ei));
  }

  sort(sorter.begin(), sorter.end());
  nassertv(sorter.size() == _entries.size());

  // Now that they're sorted, get them back.  We do this in two steps,
  // building up a temporary vector first, so we don't accidentally delete all
  // the entries when the pointers go away.
  Entries sorted_entries;
  sorted_entries.reserve(sorter.size());
  Sorter::const_iterator si;
  for (si = sorter.begin(); si != sorter.end(); ++si) {
    sorted_entries.push_back((*si)._entry);
  }

  _entries.swap(sorted_entries);
}

/**
 * Removes all the entries from the queue.
 */
void CollisionHandlerQueue::
clear_entries() {
  _entries.clear();
}

/**
 * Returns the number of CollisionEntries detected last pass.
 */
int CollisionHandlerQueue::
get_num_entries() const {
  return _entries.size();
}

/**
 * Returns the nth CollisionEntry detected last pass.
 */
CollisionEntry *CollisionHandlerQueue::
get_entry(int n) const {
  nassertr(n >= 0 && n < (int)_entries.size(), nullptr);
  return _entries[n];
}

/**
 *
 */
void CollisionHandlerQueue::
output(std::ostream &out) const {
  out << "CollisionHandlerQueue, " << _entries.size() << " entries";
}

/**
 *
 */
void CollisionHandlerQueue::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "CollisionHandlerQueue, " << _entries.size() << " entries:\n";

  Entries::const_iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    (*ei)->write(out, indent_level + 2);
  }
}
