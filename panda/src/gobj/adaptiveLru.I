/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file adaptiveLru.I
 * @author drose
 * @date 2008-09-03
 */

/**
 * Returns the total size of all objects currently active on the LRU.
 */
INLINE size_t AdaptiveLru::
get_total_size() const {
  LightMutexHolder holder(_lock);
  return _total_size;
}

/**
 * Returns the max size of all objects that are allowed to be active on the
 * LRU.
 */
INLINE size_t AdaptiveLru::
get_max_size() const {
  LightMutexHolder holder(_lock);
  return _max_size;
}

/**
 * Changes the max size of all objects that are allowed to be active on the
 * LRU.
 *
 * If the size is (size_t)-1, there is no limit.
 */
INLINE void AdaptiveLru::
set_max_size(size_t max_size) {
  LightMutexHolder holder(_lock);
  _max_size = max_size;
  if (_total_size > _max_size) {
    do_evict_to(_max_size, false);
  }
}

/**
 * Evicts a sequence of objects if the queue is full.
 */
INLINE void AdaptiveLru::
consider_evict() {
  LightMutexHolder holder(_lock);
  if (_total_size > _max_size) {
    do_evict_to(_max_size, false);
  }
}

/**
 * Evicts a sequence of objects until the queue fits within the indicated
 * target size, regardless of its normal max size.
 */
INLINE void AdaptiveLru::
evict_to(size_t target_size) {
  LightMutexHolder holder(_lock);
  if (_total_size > target_size) {
    do_evict_to(target_size, true);
  }
}

/**
 * Checks that the LRU is internally self-consistent.  Returns true if
 * successful, false if there is some problem.
 */
INLINE bool AdaptiveLru::
validate() {
  LightMutexHolder holder(_lock);
  return do_validate();
}

/**
 * Specifies the weight value used to compute the exponential moving average.
 */
INLINE void AdaptiveLru::
set_weight(PN_stdfloat weight) {
  _weight = weight;
}

/**
 * Returns the weight value used to compute the exponential moving average.
 */
INLINE PN_stdfloat AdaptiveLru::
get_weight() const {
  return _weight;
}

/**
 * Specifies the maximum number of pages the AdaptiveLru will update each
 * frame.  This is a performance optimization: keeping this number low limits
 * the impact of the AdaptiveLru's adaptive algorithm.
 */
INLINE void AdaptiveLru::
set_max_updates_per_frame(int max_updates_per_frame) {
  _max_updates_per_frame = max_updates_per_frame;
}

/**
 * Returns the maximum number of pages the AdaptiveLru will update each frame.
 */
INLINE int AdaptiveLru::
get_max_updates_per_frame() const {
  return _max_updates_per_frame;
}

/**
 *
 */
INLINE PN_stdfloat AdaptiveLru::
calculate_exponential_moving_average(PN_stdfloat value, PN_stdfloat average) const {
  return ((value - average) * _weight) + average;
}

/**
 * Returns the LRU that manages this page, or NULL if it is not currently
 * managed by any LRU.
 */
INLINE AdaptiveLru *AdaptiveLruPage::
get_lru() const {
  return _lru;
}

/**
 * Removes the page from its AdaptiveLru.
 */
INLINE void AdaptiveLruPage::
dequeue_lru() {
  enqueue_lru(nullptr);
}

/**
 * To be called when the page is used; this will move it to the tail of the
 * AdaptiveLru queue it is already on.
 *
 * This method is const because it's not technically modifying the contents of
 * the page itself.
 */
INLINE void AdaptiveLruPage::
mark_used_lru() const {
  if (_lru != nullptr) {
    ((AdaptiveLruPage *)this)->mark_used_lru(_lru);
  }
}

/**
 * To be called when the page is used; this will move it to the tail of the
 * specified AdaptiveLru queue.
 */
INLINE void AdaptiveLruPage::
mark_used_lru(AdaptiveLru *lru) {
  enqueue_lru(lru);
}

/**
 * Returns the size of this page as reported to the LRU, presumably in bytes.
 */
INLINE size_t AdaptiveLruPage::
get_lru_size() const {
  return _lru_size;
}

/**
 * Specifies the size of this page, presumably in bytes, although any unit is
 * possible.
 */
INLINE void AdaptiveLruPage::
set_lru_size(size_t lru_size) {
  if (_lru != nullptr) {
    LightMutexHolder holder(_lru->_lock);
    _lru->_total_size -= _lru_size;
    _lru->_total_size += lru_size;
    _lru_size = lru_size;
  } else {
    _lru_size = lru_size;
  }
}
