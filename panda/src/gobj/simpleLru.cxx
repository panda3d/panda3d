/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file simpleLru.cxx
 * @author drose
 * @date 2007-05-11
 */

#include "simpleLru.h"
#include "indent.h"

using std::ostream;

// We define this as a reference to an allocated object, instead of as a
// concrete object, so that it won't get destructed when the program exits.
// (If it did, there would be an ordering issue between it and the various
// concrete SimpleLru objects which reference it.)
LightMutex &SimpleLru::_global_lock = *new LightMutex;

/**
 *
 */
SimpleLru::
SimpleLru(const std::string &name, size_t max_size) :
  LinkedListNode(true),
  Namable(name)
{
  _total_size = 0;
  _max_size = max_size;
  _active_marker = new SimpleLruPage(0);
}

/**
 *
 */
SimpleLru::
~SimpleLru() {
  delete _active_marker;

#ifndef NDEBUG
  // We're shutting down.  Force-remove everything remaining, but don't
  // explicitly evict it (that would force vertex buffers to write themselves
  // to disk unnecessarily).
  while (_next != (LinkedListNode *)this) {
    nassertv(_next != nullptr);
    ((SimpleLruPage *)_next)->_lru = nullptr;
    ((SimpleLruPage *)_next)->remove_from_list();
  }
#endif
}

/**
 * Adds the page to the LRU for the first time, or marks it recently-accessed
 * if it has already been added.
 *
 * If lru is NULL, it means to remove this page from its LRU.
 */
void SimpleLruPage::
enqueue_lru(SimpleLru *lru) {
  LightMutexHolder holder(SimpleLru::_global_lock);

  if (_lru == lru) {
    if (_lru != nullptr) {
      remove_from_list();
      insert_before(_lru);
    }
    return;
  }

  if (_lru != nullptr) {
    remove_from_list();
    _lru->_total_size -= _lru_size;
    _lru = nullptr;
  }

  _lru = lru;

  if (_lru != nullptr) {
    _lru->_total_size += _lru_size;
    insert_before(_lru);
  }

  // Let's not automatically evict pages; instead, we'll evict only on an
  // explicit epoch test.  _lru->consider_evict();
}

/**
 * Returns the total size of the pages that were enqueued since the last call
 * to begin_epoch().
 */
size_t SimpleLru::
count_active_size() const {
  LightMutexHolder holder(_global_lock);
  size_t total = 0;

  LinkedListNode *node = _prev;
  while (node != _active_marker && node != this) {
    total += ((SimpleLruPage *)node)->get_lru_size();
    node = ((SimpleLruPage *)node)->_prev;
  }

  return total;
}

/**
 *
 */
void SimpleLru::
output(ostream &out) const {
  LightMutexHolder holder(_global_lock);
  out << "SimpleLru " << get_name()
      << ", " << _total_size << " of " << _max_size;
}

/**
 *
 */
void SimpleLru::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << ":\n";

  // We write out the list backwards.  Things we write out first are the
  // freshest in the LRU.  Things at the end of the list will be the next to
  // be evicted.

  LightMutexHolder holder(_global_lock);
  LinkedListNode *node = _prev;
  while (node != _active_marker && node != this) {
    SimpleLruPage *page = (SimpleLruPage *)node;
    indent(out, indent_level + 2) << *page << " (active)\n";
    node = page->_prev;
  }
  if (node == _active_marker) {
    node = _active_marker->_prev;
    while (node != this) {
      SimpleLruPage *page = (SimpleLruPage *)node;
      indent(out, indent_level + 2) << *page << "\n";
      node = page->_prev;
    }
  }

#ifndef NDEBUG
  ((SimpleLru *)this)->do_validate();
#endif
}

/**
 * Evicts pages until the LRU is within the indicated size.  Assumes the lock
 * is already held.  If hard_evict is false, does not evict "active" pages
 * that were added within this epoch.
 */
void SimpleLru::
do_evict_to(size_t target_size, bool hard_evict) {
  if (_next == this) {
    // Nothing in the queue.
    return;
  }

  // Store the current end of the list.  If pages re-enqueue themselves during
  // this traversal, we don't want to visit them twice.
  SimpleLruPage *end = (SimpleLruPage *)_prev;

  // Now walk through the list.
  SimpleLruPage *node = (SimpleLruPage *)_next;
  while (_total_size > target_size) {
    SimpleLruPage *next = (SimpleLruPage *)node->_next;

    // We must release the lock while we call evict_lru().
    _global_lock.unlock();
    node->evict_lru();
    _global_lock.lock();

    if (node == end || node == _prev) {
      // If we reach the original tail of the list, stop.
      return;
    }
    if (!hard_evict && node == _active_marker) {
      // Also stop if we reach the active marker.  Nodes beyond this were
      // added within this epoch.
      return;
    }
    node = next;
  }
}

/**
 * Checks that the LRU is internally consistent.  Assume the lock is already
 * held.
 */
bool SimpleLru::
do_validate() {
  size_t total = 0;

  LinkedListNode *node = _next;
  while (node != this) {
    total += ((SimpleLruPage *)node)->get_lru_size();
    node = ((SimpleLruPage *)node)->_next;
  }

  return (total == _total_size);
}

/**
 *
 */
SimpleLruPage::
~SimpleLruPage() {
  if (_lru != nullptr) {
    dequeue_lru();
  }
}

/**
 * Evicts the page from the LRU.  Called internally when the LRU determines
 * that it is full.  May also be called externally when necessary to
 * explicitly evict the page.
 *
 * It is legal for this method to either evict the page as requested, do
 * nothing (in which case the eviction will be requested again at the next
 * epoch), or requeue itself on the tail of the queue (in which case the
 * eviction will be requested again much later).
 */
void SimpleLruPage::
evict_lru() {
  dequeue_lru();
}

/**
 *
 */
void SimpleLruPage::
output(ostream &out) const {
  out << "page " << this << ", " << _lru_size;
}

/**
 *
 */
void SimpleLruPage::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}
