// Filename: collisionHandlerQueue.cxx
// Created by:  drose (27Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "collisionHandlerQueue.h"
#include "config_collide.h"

TypeHandle CollisionHandlerQueue::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerQueue::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CollisionHandlerQueue::
CollisionHandlerQueue() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerQueue::begin_group
//       Access: Public, Virtual
//  Description: Will be called by the CollisionTraverser before a new
//               traversal is begun.  It instructs the handler to
//               reset itself in preparation for a number of
//               CollisionEntries to be sent.
////////////////////////////////////////////////////////////////////
void CollisionHandlerQueue::
begin_group() {
  _entries.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerQueue::add_entry
//       Access: Public, Virtual
//  Description: Called between a begin_group() .. end_group()
//               sequence for each collision that is detected.
////////////////////////////////////////////////////////////////////
void CollisionHandlerQueue::
add_entry(CollisionEntry *entry) {
  nassertv(entry != (CollisionEntry *)NULL);
  _entries.push_back(entry);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerQueue::get_num_entries
//       Access: Public
//  Description: Returns the number of CollisionEntries detected last
//               pass.
////////////////////////////////////////////////////////////////////
int CollisionHandlerQueue::
get_num_entries() const {
  return _entries.size();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionHandlerQueue::get_entry
//       Access: Public
//  Description: Returns the nth CollisionEntry detected last pass.
////////////////////////////////////////////////////////////////////
CollisionEntry *CollisionHandlerQueue::
get_entry(int n) const {
  nassertr(n >= 0 && n < (int)_entries.size(), NULL);
  return _entries[n];
}
