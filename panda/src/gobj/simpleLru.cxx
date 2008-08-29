// Filename: simpleLru.cxx
// Created by:  drose (11May07)
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

#include "simpleLru.h"
#include "clockObject.h"

// We define this as a reference to an allocated object, instead of as
// a concrete object, so that it won't get destructed when the program
// exits.  (If it did, there would be an ordering issue between it and
// the various concrete SimpleLru objects which reference it.)
Mutex &SimpleLru::_global_lock = *new Mutex;

////////////////////////////////////////////////////////////////////
//     Function: SimpleLru::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
SimpleLru::
SimpleLru(const string &name, size_t max_size) : 
  LinkedListNode(true), 
  Namable(name)
{
  _total_size = 0;
  _max_size = max_size;
  _active_marker = new SimpleLruPage(0);
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLru::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
SimpleLru::
~SimpleLru() {
  delete _active_marker;

#ifndef NDEBUG
  // We're shutting down.  Force-remove everything remaining, but
  // don't explicitly evict it (that would force vertex buffers to
  // write themselves to disk unnecessarily).
  while (_next != (LinkedListNode *)this) {
    nassertv(_next != (LinkedListNode *)NULL);
    ((SimpleLruPage *)_next)->_lru = NULL;
    ((SimpleLruPage *)_next)->remove_from_list();
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLruPage::enqueue_lru
//       Access: Published
//  Description: Adds the page to the tail of the SimpleLru.  When it
//               reaches the head, it will be the next to be evicted.
////////////////////////////////////////////////////////////////////
void SimpleLruPage::
enqueue_lru(SimpleLru *lru) {
  MutexHolder holder(SimpleLru::_global_lock);

  if (_lru == lru) {
    if (_lru != (SimpleLru *)NULL) {
      remove_from_list();
      insert_before(_lru);
    }
    return;
  }

  if (_lru != (SimpleLru *)NULL) {
    remove_from_list();
    _lru->_total_size -= _lru_size;
    _lru = NULL;
  }

  _lru = lru;

  if (_lru != (SimpleLru *)NULL) {
    _lru->_total_size += _lru_size;
    insert_before(_lru);
  }

  // Let's not automatically evict pages; instead, we'll evict only on
  // an explicit epoch test.
  //  _lru->consider_evict();
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLru::count_active_size
//       Access: Published
//  Description: Returns the total size of the pages that were
//               enqueued since the last call to begin_epoch().
////////////////////////////////////////////////////////////////////
size_t SimpleLru::
count_active_size() const {
  MutexHolder holder(_global_lock);
  size_t total = 0;

  LinkedListNode *node = _prev;
  while (node != _active_marker && node != this) {
    total += ((SimpleLruPage *)node)->get_lru_size();
    node = ((SimpleLruPage *)node)->_prev;
  }

  return total;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLru::do_evict_to
//       Access: Private
//  Description: Evicts pages until the LRU is within the indicated
//               size.  Assumes the lock is already held.  If
//               hard_evict is false, does not evict "active" pages
//               that were added within this epoch.
////////////////////////////////////////////////////////////////////
void SimpleLru::
do_evict_to(size_t target_size, bool hard_evict) {
  if (_next == this) {
    // Nothing in the queue.
    return;
  }

  // Store the current end of the list.  If pages re-enqueue
  // themselves during this traversal, we don't want to visit them
  // twice.
  SimpleLruPage *end = (SimpleLruPage *)_prev;

  // Now walk through the list.
  SimpleLruPage *node = (SimpleLruPage *)_next;
  while (_total_size > target_size) {
    SimpleLruPage *next = (SimpleLruPage *)node->_next;

    // We must release the lock while we call evict_lru().
    _global_lock.release();
    node->evict_lru();
    _global_lock.lock();

    if (node == end || node == _prev) {
      // If we reach the original tail of the list, stop.
      return;
    }
    if (!hard_evict && node == _active_marker) {
      // Also stop if we reach the active marker.  Nodes beyond this
      // were added within this epoch.
      return;
    }
    node = next;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLru::do_validate_size
//       Access: Private
//  Description: Checks that _total_size is consistent.  Assume the
//               lock is already held.
////////////////////////////////////////////////////////////////////
bool SimpleLru::
do_validate_size() {
  size_t total = 0;

  LinkedListNode *node = _next;
  while (node != this) {
    total += ((SimpleLruPage *)node)->get_lru_size();
    node = ((SimpleLruPage *)node)->_next;
  }

  return (total == _total_size);
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLruPage::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
SimpleLruPage::
~SimpleLruPage() {
  if (_lru != NULL) {
    dequeue_lru();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLruPage::evict_lru
//       Access: Published, Virtual
//  Description: Evicts the page from the LRU.  Called internally when
//               the LRU determines that it is full.  May also be
//               called externally when necessary to explicitly evict
//               the page.
//
//               It is legal for this method to either evict the page
//               as requested, do nothing (in which case the eviction
//               will be requested again at the next epoch), or
//               requeue itself on the tail of the queue (in which
//               case the eviction will be requested again much
//               later).
////////////////////////////////////////////////////////////////////
void SimpleLruPage::
evict_lru() {
  dequeue_lru();
}
