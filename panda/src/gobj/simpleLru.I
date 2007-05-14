// Filename: simpleLru.I
// Created by:  drose (11May07)
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


////////////////////////////////////////////////////////////////////
//     Function: SimpleLru::get_total_size
//       Access: Published
//  Description: Returns the total size of all objects currently
//               active on the LRU.
////////////////////////////////////////////////////////////////////
INLINE size_t SimpleLru::
get_total_size() const {
  return _total_size;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLru::get_max_size
//       Access: Published
//  Description: Returns the max size of all objects that are allowed
//               to be active on the LRU.
////////////////////////////////////////////////////////////////////
INLINE size_t SimpleLru::
get_max_size() const {
  return _max_size;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLru::set_max_size
//       Access: Published
//  Description: Changes the max size of all objects that are allowed
//               to be active on the LRU.
//
//               If the size is (size_t)-1, there is no limit.
////////////////////////////////////////////////////////////////////
INLINE void SimpleLru::
set_max_size(size_t max_size) {
  _max_size = max_size;
  consider_evict();
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLru::consider_evict
//       Access: Published
//  Description: Evicts a sequence of objects if the queue is full.
////////////////////////////////////////////////////////////////////
INLINE void SimpleLru::
consider_evict() {
  if (_total_size > _max_size) {
    do_evict();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLru::begin_epoch
//       Access: Published
//  Description: Marks the end of the previous epoch and the beginning
//               of the next one.  This will evict any objects that
//               are pending eviction, and also update any internal
//               bookkeeping.
////////////////////////////////////////////////////////////////////
INLINE void SimpleLru::
begin_epoch() {
  consider_evict();
  _active_marker->enqueue_lru(this);
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLruPage::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE SimpleLruPage::
SimpleLruPage(size_t lru_size) :
  _lru(NULL),
  _lru_size(lru_size)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLruPage::Copy Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE SimpleLruPage::
SimpleLruPage(const SimpleLruPage &copy) :
  _lru(NULL),
  _lru_size(copy._lru_size)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLruPage::Copy Assignment Operator
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void SimpleLruPage::
operator = (const SimpleLruPage &copy) {
  set_lru_size(copy.get_lru_size());
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLruPage::enqueue_lru
//       Access: Published
//  Description: Returns the LRU that manages this page, or NULL if it
//               is not currently managed by any LRU.
////////////////////////////////////////////////////////////////////
INLINE SimpleLru *SimpleLruPage::
get_lru() const {
  return _lru;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLruPage::enqueue_lru
//       Access: Published
//  Description: Adds the page to the tail of the SimpleLru.  When it
//               reaches the head, it will be the next to be evicted.
////////////////////////////////////////////////////////////////////
INLINE void SimpleLruPage::
enqueue_lru(SimpleLru *lru) {
  dequeue_lru();
  _lru = lru;
  _lru->_total_size += _lru_size;
  insert_before(_lru);

  // Let's not automatically evict pages; instead, we'll evict only on
  // an explicit epoch test.
  //  _lru->consider_evict();
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLruPage::dequeue_lru
//       Access: Published
//  Description: Removes the page from its SimpleLru.
////////////////////////////////////////////////////////////////////
INLINE void SimpleLruPage::
dequeue_lru() {
  if (_lru != (SimpleLru *)NULL) {
    remove_from_list();
    _lru->_total_size -= _lru_size;
    _lru = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLruPage::mark_used_lru
//       Access: Published
//  Description: To be called when the page is used; this will move it
//               to the tail of the SimpleLru queue it is already on.
////////////////////////////////////////////////////////////////////
INLINE void SimpleLruPage::
mark_used_lru() {
  if (_lru != (SimpleLru *)NULL) {
    mark_used_lru(_lru);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLruPage::mark_used_lru
//       Access: Published
//  Description: To be called when the page is used; this will move it
//               to the tail of the specified SimpleLru queue.
////////////////////////////////////////////////////////////////////
INLINE void SimpleLruPage::
mark_used_lru(SimpleLru *lru) {
  dequeue_lru();
  enqueue_lru(lru);
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLruPage::get_lru_size
//       Access: Published
//  Description: Returns the size of this page as reported to the LRU,
//               presumably in bytes.
////////////////////////////////////////////////////////////////////
INLINE size_t SimpleLruPage::
get_lru_size() const {
  return _lru_size;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleLruPage::set_lru_size
//       Access: Published
//  Description: Specifies the size of this page, presumably in bytes,
//               although any units is possible.
////////////////////////////////////////////////////////////////////
INLINE void SimpleLruPage::
set_lru_size(size_t lru_size) {
  if (_lru != (SimpleLru *)NULL) {
    _lru->_total_size -= _lru_size;
    _lru->_total_size += lru_size;
  }
  _lru_size = lru_size;
}
