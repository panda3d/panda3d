/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file simpleLru.I
 * @author drose
 * @date 2007-05-11
 */

/**
 * Returns the total size of all objects currently active on the LRU.
 */
INLINE size_t SimpleLru::
get_total_size() const {
  LightMutexHolder holder(_global_lock);
  return _total_size;
}

/**
 * Returns the max size of all objects that are allowed to be active on the
 * LRU.
 */
INLINE size_t SimpleLru::
get_max_size() const {
  LightMutexHolder holder(_global_lock);
  return _max_size;
}

/**
 * Changes the max size of all objects that are allowed to be active on the
 * LRU.
 *
 * If the size is (size_t)-1, there is no limit.
 */
INLINE void SimpleLru::
set_max_size(size_t max_size) {
  LightMutexHolder holder(_global_lock);
  _max_size = max_size;
  if (_total_size > _max_size) {
    do_evict_to(_max_size, false);
  }
}

/**
 * Evicts a sequence of objects if the queue is full.
 */
INLINE void SimpleLru::
consider_evict() {
  LightMutexHolder holder(_global_lock);
  if (_total_size > _max_size) {
    do_evict_to(_max_size, false);
  }
}

/**
 * Evicts a sequence of objects until the queue fits within the indicated
 * target size, regardless of its normal max size.
 */
INLINE void SimpleLru::
evict_to(size_t target_size) {
  LightMutexHolder holder(_global_lock);
  if (_total_size > target_size) {
    do_evict_to(target_size, true);
  }
}

/**
 * Marks the end of the previous epoch and the beginning of the next one.
 * This will evict any objects that are pending eviction, and also update any
 * internal bookkeeping.
 */
INLINE void SimpleLru::
begin_epoch() {
  consider_evict();
  _active_marker->enqueue_lru(this);
}

/**
 * Checks that the LRU is internally self-consistent.  Returns true if
 * successful, false if there is some problem.
 */
INLINE bool SimpleLru::
validate() {
  LightMutexHolder holder(_global_lock);
  return do_validate();
}

/**
 *
 */
INLINE SimpleLruPage::
SimpleLruPage(size_t lru_size) :
  _lru(nullptr),
  _lru_size(lru_size)
{
}

/**
 *
 */
INLINE SimpleLruPage::
SimpleLruPage(const SimpleLruPage &copy) :
  _lru(nullptr),
  _lru_size(copy._lru_size)
{
}

/**
 *
 */
INLINE void SimpleLruPage::
operator = (const SimpleLruPage &copy) {
  set_lru_size(copy.get_lru_size());
}

/**
 * Returns the LRU that manages this page, or NULL if it is not currently
 * managed by any LRU.
 */
INLINE SimpleLru *SimpleLruPage::
get_lru() const {
  LightMutexHolder holder(SimpleLru::_global_lock);
  return _lru;
}

/**
 * Removes the page from its SimpleLru.
 */
INLINE void SimpleLruPage::
dequeue_lru() {
  LightMutexHolder holder(SimpleLru::_global_lock);

  if (_lru != nullptr) {
    remove_from_list();
    _lru->_total_size -= _lru_size;
    _lru = nullptr;
  }
}

/**
 * To be called when the page is used; this will move it to the tail of the
 * SimpleLru queue it is already on.
 *
 * This method is const because it's not technically modifying the contents of
 * the page itself.
 */
INLINE void SimpleLruPage::
mark_used_lru() const {
  if (_lru != nullptr) {
    ((SimpleLruPage *)this)->mark_used_lru(_lru);
  }
}

/**
 * To be called when the page is used; this will move it to the tail of the
 * specified SimpleLru queue.
 */
INLINE void SimpleLruPage::
mark_used_lru(SimpleLru *lru) {
  enqueue_lru(lru);
}

/**
 * Returns the size of this page as reported to the LRU, presumably in bytes.
 */
INLINE size_t SimpleLruPage::
get_lru_size() const {
  return _lru_size;
}

/**
 * Specifies the size of this page, presumably in bytes, although any unit is
 * possible.
 */
INLINE void SimpleLruPage::
set_lru_size(size_t lru_size) {
  LightMutexHolder holder(SimpleLru::_global_lock);
  if (_lru != nullptr) {
    _lru->_total_size -= _lru_size;
    _lru->_total_size += lru_size;
    _lru_size = lru_size;
  } else {
    _lru_size = lru_size;
  }
}
